#! /bin/bash

print_help()
{
    if [[ $# == 1 ]]; then
        echo -e "\n\t\e[31m$1\e[0m"
    fi

    echo "
    Usage:
    ./run.sh -s  ---- build for server
    ./run.sh -c  ---- build for client
    "
}

run_server()
{
    ./build/server
}

run_client()
{
    ./build/client
}

if [[ $# != 1 ]]; then
    print_help "arg number is invald: $#"
    exit
fi

if [[ "$1" == '-c' ]]; then
    run_client
elif [[ "$1" == '-s' ]]; then
    run_server
else
    print_help "invald arg: $1"
fi