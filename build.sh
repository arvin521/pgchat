#! /bin/bash

print_help()
{
    if [[ $# == 1 ]]; then
        echo -e "\n\t\e[31m$1\e[0m"
    fi

    echo "
    Usage:
    ./bulid.sh -s  ---- build for server
    ./bulid.sh -c  ---- build for client
    ./bulid.sh     ---- build for server and client
    "
}

build_server()
{
    gcc src/server/server.c -o build/server
}

build_client()
{
    gcc src/client/client.c -o build/client
}

if [[ $# != 1 && $# != 0 ]]; then
    print_help "arg number is invald: $#"
    exit
fi

PWD=`pwd`
export C_INCLUDE_PATH=C_INCLUDE_PATH:$PWD

if [[ $# == 0 ]]; then
    build_client
    build_server
elif [[ "$1" == '-c' ]]; then
    build_client
elif [[ "$1" == '-s' ]]; then
    build_server
else
    print_help "invald arg: $1"
fi