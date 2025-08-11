#!/bin/bash

set -e

exit_help() {
    echo "Usage: $0 <task> [optional]"
    echo "task: "
    echo "  update              Update this repository from github."
    echo "  upload [message]    Upload changes to github."
    exit 1 # Exit with a non-zero status code to indicate an error
}

if [ "$#" -eq 0 ]; then
    echo "Error: No arguments provided."
    exit_help
fi

case "$1" in
    "update")
        git status --porcelain
        if [ -z $? ]; then
            echo "There changes to deal before update."
            exit 1
        fi
        git pull
        echo "Done update."
        ;;
    "upload")
        echo "Upload changes to github repository."
        # check position is in valid git repository
        if [ ! -d ".git" ]; then
            echo "This is currently not git repository. Return now"
            exit 1
        fi
        # if there is a second arguments
        if [ -z $2 ]; then
            commit_message=$2
        else
            # default commit message
            commit_message="Update repository at $(date '+%x %R')"
        fi
        git add .
        git commit -m "$commit_message"
        if [ -n $? ]; then
            git push
        else
            echo "Nothing is changed."
            exit 1
        fi
        echo "Changes uploaded"
        ;;
        
    *)
        exit_help
        ;;
esac
