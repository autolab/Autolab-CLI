
#!/bin/bash

if [[ $EUID -ne 0 ]]; then  
    echo "This script must be run as root (or with sudo)"
    exit 1
fi

sudo cp autocomplete/autolab /etc/bash_completion.d/
. /etc/bash_completion.d/autolab