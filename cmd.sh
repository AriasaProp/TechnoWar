#!/bin/bash

set -e

exit_help() {
  echo "Usage: $0 <task> [optional]"
  echo "task: "
  echo "  format              formating code files."
  echo "  ttool [target]      Quick test for tool test. if [target] defined it will run spesific test."
  echo "  update              Update this repository from github."
  echo "  artifact            Download Artifact from github."
  echo "  up [message]        Upload changes to github."
  exit 1 # Exit with a non-zero status code to indicate an error
}

if [ "$#" -eq 0 ]; then
  echo "Error: No arguments provided."
  exit_help
fi

case "$1" in
  "format")
    find . -regex '.*/.*\.\(c\|h\|java\)$' | xargs clang-format -style=file -i
    ;;
  "ttool")
    cd tools
    cmake . -DDO_TEST=1
    if [ ! -z $2 ]; then
      cmake --build . --target $2
    else
      cmake --build .
    fi
    ctest -V
    cd ..
    ;;
  "update")
    git status --porcelain
    if [ -z $? ]; then
      echo "There changes to deal before update."
      exit 1
    fi
    git pull
    echo "Done update."
    ;;
  "artifact")
    gh run download -n Android-APK
    mv release/* /../../storage/emulated/0/
    rm -rf debug
    rm -rf release
    ;;
  "up")
    echo "Upload changes to github repository."
    # check position is in valid git repository
    if [ ! -d ".git" ]; then
      echo "This is currently not git repository. Return now"
      exit 1
    fi
    # if there are arguments after "upload"
    if [ "$#" -gt 1 ]; then
      shift # remove "upload" from arguments list
      commit_message = "$*" # join the rest of the arguments
    else
      # default commit message
      commit_message = "Update repository at $(date '+%x %R')"
    fi
    git add .
    git commit -m "$commit_message"
    if [ $? -eq 0 ]; then
      git push
      echo "Changes uploaded"
    else
      echo "Nothing to commit, or commit failed."
    fi
    sleep 1
    gh run watch $(gh run list --limit 1 --json databaseId -q '.[0].databaseId')
    git pull
    ;;
  *)
    exit_help
  ;;
esac
