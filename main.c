#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <getopt.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>


#include <chout.h>
#include <string_utils.h>
#include <htable.h>
#include <configuration.h>

#include <ping_list.h>
#include <pscheck.h>
#include <job.h>
#include <pinger_config.h>
#include <pinger_signal.h>
#include <pinger_report.h>
#include <pinger_archive.h>

#include <pthreadpool.h>

#include <uttime.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define NPING "pinger.default.nping"
#define RPING "pinger.ping.runs"
#define TPSIZE "pinger.threadpool.size"
#define REPORT_ALARM "pinger.report.alarm"
#define OUTDIR "pinger.report.dir"
#define ARCHIVE_TOOL "pinger.archive.tool"

typedef struct _pinger_args {
	char** hosts;
	char* n_ping;
	int runs;
	int size;
	int tpsize;
	int alrm_sec;
	char* report_dir;
	char* archive_sh;
} pinger_args;

void usage(char* prog_name, char* message);
int isIpAddress(char* arg);
pinger_args* check_argv(int argc, char* argv[], htable* conf);
void version(char* prog_name);

int prepare_report_fs(char* outdir);

void chunkdestroy(void* chunklist);
void strdestroy(void* str);


int main(int argc, char *argv[]) {
	int TERM = 0;
	char* conf_path[3] = {"../", argv[0], ".conf"};
	char* conf_path_s = string_builder(conf_path, 3, NULL);

	htable* conf = load_configuration(conf_path_s);
	free(conf_path_s);
	pinger_args* args = check_argv(argc, argv, conf);
	conf->destroy(conf);
	if(args == NULL) {
		exit(1);
	}

	if(prepare_report_fs(args->report_dir) < 0) {
		perror("prepare_report_fs");
		exit(1);
	}

	htable* hosts = new_htable(strdestroy, chunkdestroy);
	for(int i=0; i<args->size; i++) {
		hosts->put(hosts, args->hosts[i], new_chunk_list(), strlen(args->hosts[i]));
	}

	report_args_t* report_args = malloc(sizeof(report_args_t));
	report_args->hosts = hosts;
	report_args->reportout = args->report_dir;

	archive_args_t* archive_args = malloc(sizeof(archive_args_t));

	strncpy(archive_args->dir, args->report_dir, strlen(args->report_dir));
	strncpy(archive_args->archive_sh, args->archive_sh, strlen(args->archive_sh));

	handler_args_t* handler_arg = malloc(sizeof(handler_args_t));
	handler_arg->sighandl = malloc(sizeof(sigs_handling));
	handler_arg->termflag = &TERM;
	handler_arg->alarm_handler = malloc(sizeof(alarmh_t));
	handler_arg->alarm_handler->alarm_r = do_report;
	handler_arg->alarm_handler->arg_r = report_args;
	handler_arg->alarm_handler->alrm_sec = args->alrm_sec;
	handler_arg->alarm_handler->eachn_r = malloc(3 * sizeof(alarmh_t*));
	handler_arg->alarm_handler->eachn_r[0] = malloc(sizeof(alarmh_t));
	handler_arg->alarm_handler->eachn_r[0]->alarm_r = archive;
	handler_arg->alarm_handler->eachn_r[0]->arg_r = archive_args;
	handler_arg->alarm_handler->eachn_r[0]->alrm_sec = 10;
	handler_arg->alarm_handler->eachn_r[1] = malloc(sizeof(alarmh_t));
	handler_arg->alarm_handler->eachn_r[1]->alarm_r = reset;
	handler_arg->alarm_handler->eachn_r[1]->arg_r = hosts;
	handler_arg->alarm_handler->eachn_r[1]->alrm_sec = 360;
	handler_arg->alarm_handler->eachn_r[2] = NULL;
	maskall(handler_arg->sighandl->sigset, &handler_arg->sighandl->sigs_handlers);

	short circletp = args->runs == -1;
	int ntasks = args->runs == -1 ? args->size : args->runs * args->size;

	pthreadpool_t* tp = new_threadpool(args->tpsize, &TERM, circletp);
	pinger_thread_args** r_args = malloc(sizeof(pinger_thread_args*) * ntasks);
	int t = 0, h = 0;
	while(t < ntasks) {
		r_args[h] = malloc(sizeof(pinger_thread_args));
		r_args[h]->host = args->hosts[h];
		r_args[h]->n_ping = args->n_ping;
		r_args[h]->runs = args->runs;
		r_args[h]->hosts_data = hosts;
		tp->add_task(tp, doping_r, r_args[h]);
		t++;
		h = (h + 1) % args->size;
	}
	tp->start(tp);
	
	sigemptyset(&handler_arg->sighandl->sigset);
	sigaddset(&handler_arg->sighandl->sigset, SIGTERM);
	sigaddset(&handler_arg->sighandl->sigset, SIGQUIT);
	sigaddset(&handler_arg->sighandl->sigset, SIGSTOP);
	sigaddset(&handler_arg->sighandl->sigset, SIGINT);
	sigaddset(&handler_arg->sighandl->sigset, SIGALRM);
	sigaddset(&handler_arg->sighandl->sigset, SIGSEGV);
	sigaddset(&handler_arg->sighandl->sigset, SIGUSR1);
	pthread_sigmask(SIG_SETMASK, &handler_arg->sighandl->sigset, NULL);

	alarm(args->alrm_sec);
	while(!TERM) {
		handling_r(handler_arg);
	}
	int i = 0;
	while(handler_arg->alarm_handler->eachn_r[i] != NULL) {
		free(handler_arg->alarm_handler->eachn_r[i]);
		i++;
	}
	free(handler_arg->alarm_handler->eachn_r);
	free(handler_arg->alarm_handler);
	free(handler_arg->sighandl);
	free(handler_arg);

	if(!circletp) tp->wait(tp);
	else tp->stop(tp);

	destroy_pthreadpool(tp);
	free(r_args);
	hosts->destroy(hosts);
	free(report_args);
	free(archive_args);
	free(args->hosts);
	free(args->n_ping);
	free(args->report_dir);
	free(args->archive_sh);
	free(args);
	return 0;
}


int prepare_report_fs(char* outdir) {
	char archived_path[128];
	snprintf(archived_path, 128, "%s/archived/", outdir);
	struct stat st = {0};

	if (stat(archived_path, &st) == -1) {
	    int mkdir_r = mkdir(archived_path, 0777);
	    return mkdir_r;
	} else {
		return 0;
	}
}


int isIpAddress(char* arg) {
	char ipBytes[4];
	int i=0;
	while(i < strlen(arg)) {
		int j = i;
		int b = 0;
		while(j < strlen(arg) && arg[j] != '.') {
			ipBytes[b++] = arg[j++];
		}
		ipBytes[b] = '\0';
		int intByte = atoi(ipBytes);
		if(!(intByte >= 0 && intByte < 256)) return 0;
		i = j + 1;
	}

	return 1;
}


void version(char* prog_name) {
	fprintf(stderr, "%s version: %d.%d.%d\n", prog_name, PINGER_VERSION_MAJOR, PINGER_VERSION_MINOR, PINGER_VERSION_PATCH);
	fprintf(stderr, "\n");
}


void usage(char* prog_name, char* message) {
	if(message != NULL) {
		fprintf(stderr, "%s\n", message);
	}
	version(prog_name);
	fprintf(stderr, "Use %s [-crtlh] host_list... \n", prog_name);
	fprintf(stderr, "-c\t| number of ping packets sent\n");
	fprintf(stderr, "-r\t| number of ping runs\n");
	fprintf(stderr, "-t\t| number of threads in threadpool\n");
	fprintf(stderr, "-l\t| interval to produce report\n");
	fprintf(stderr, "-o\t| path to directory where pinger will create report files.\n");
	fprintf(stderr, "-a\t| archive script. Write your own script to archive pinger log files" \
	 "as you prefer.\n\t  It will be executed once each 10 reports (10*'-l' option value), passing as argument the timestamp.\n");
	fprintf(stderr, "-v\t| print pinger version number and exit\n");
	fprintf(stderr, "-h\t| print this message and exit\n");
	fprintf(stderr, "\nhost_list:\t host comma separated list.\n");
}


pinger_args* check_argv(int argc, char* argv[], htable* conf) {
	if(argc <= 1) return NULL;
	const char* optstring = "c:r:t:l:o:hv";
	pinger_args* args = malloc(sizeof(pinger_args));
	args->size = 0;
	char optc;
	int c_arg = 0, r_arg = 0, t_arg = 0, l_arg = 0, o_arg = 0, a_arg = 0;
	while((optc = getopt(argc, argv, optstring)) != -1) {
		switch(optc) {
			case 'c': {
				args->n_ping = strndup(optarg, strlen(optarg));
				c_arg = 1;
			} break;
			case 'r': {
				args->runs = atoi(optarg);
				r_arg = 1;
			} break;
			case 't': {
				args->tpsize = atoi(optarg);
				t_arg = 1;
			} break;
			case 'l': {
				args->alrm_sec = atoi(optarg);
				l_arg = 1;
			} break;
			case 'o': {
				args->report_dir = strndup(optarg, strlen(optarg));
				o_arg = 1;
			} break;
			case 'a': {
				args->archive_sh = strndup(optarg, strlen(optarg));
				a_arg = 1;
			} break;
			case 'h': {
				usage(argv[0], NULL);
				free(args);
				return NULL;
			} break;
			case 'v': {
				version(argv[0]);
				free(args);
				return NULL;
			} break;
			default: {
				char* errmessage = string_concat(optarg, " is not a valid option.\n");
				usage(argv[0], errmessage);
				free(errmessage);
				free(args->hosts);
				free(args);
				return NULL;
			}
		} 
	}
	if(optind > 0 && argv[optind] != NULL) {
		args->hosts = strspltc(argv[optind], ',', &args->size);
	} else {
		usage(argv[0], "Missing host argument.");
		return NULL;
	}
	if(!c_arg) {
		char* tmp = conf->get(conf, NPING, strlen(NPING));
		args->n_ping = strndup(tmp, strlen(tmp));
	}
	if(!r_arg) args->runs = atoi((char*) conf->get(conf, RPING, strlen(RPING)));
	if(!t_arg) args->tpsize = atoi((char*) conf->get(conf, TPSIZE, strlen(TPSIZE)));
	if(!l_arg) args->alrm_sec = atoi((char*) conf->get(conf, REPORT_ALARM, strlen(REPORT_ALARM)));
	if(!o_arg) {
		char* tmp = conf->get(conf, OUTDIR, strlen(OUTDIR));
		args->report_dir = strndup(tmp, strlen(tmp));
	}
	if(!a_arg) {
		char* tmp = conf->get(conf, ARCHIVE_TOOL, strlen(ARCHIVE_TOOL));
		args->archive_sh = strndup(tmp, strlen(tmp));
	}

	for(int i=1; i<argc; i++) {
		if(!isIpAddress(argv[i])) {
			char* errmessage = string_concat(argv[i], " is malformed.\n");
			usage(argv[0], errmessage);
			free(errmessage);
			return NULL;
		}
	}
	return args;
}


void chunkdestroy(void* chunklist) {
	((chunk_list*) chunklist)->destroy(((chunk_list*) chunklist));
}


void strdestroy(void* str) {
	free(str);
}
