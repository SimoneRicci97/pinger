#!/bin/sh
echo "Creating tar pinger.archive.$1.log.tar"

/bin/tar -cf out/pinger.archive.$1.log.tar out/*.log

# mv out/*.log out/archived