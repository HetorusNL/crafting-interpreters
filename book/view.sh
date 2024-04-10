#!/bin/bash

# magic options for bash to make scripts safer
set -Eeuxo pipefail

# open the website started with the next command
xdg-open localhost:8000
# run caddy with the website as a blocking call
caddy run
