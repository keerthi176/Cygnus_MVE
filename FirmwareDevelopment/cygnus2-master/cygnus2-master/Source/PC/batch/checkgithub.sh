#!/bin/sh

#
# checkgithub.sh
#
# shell script to periodically compare references in a local repository with github
# if the local repository is out of date then the updates are pulled from github and
# pushed to gitlab
#
# This script assumes the local repository includes the remote called gitlab
#

# constants
DELAY=60

echo $0 started

while true
do
  # bring remote origin ref up to date
  git remote update origin
  
  # compare local and remote refs
  UPSTREAM=${1:-'@{u}'}
  LOCAL=$(git rev-parse @)
  REMOTE=$(git rev-parse "$UPSTREAM")
  BASE=$(git merge-base @ "$UPSTREAM")
  #echo $LOCAL
  #echo $REMOTE
  #echo $BASE
 
  if [ $LOCAL = $REMOTE ]; then
      echo "Up-to-date"
  elif [ $LOCAL = $BASE ]; then
      echo "Need to pull"
      date
      
      # pull from github
      git pull
      
      # push to gitlab
      git remote update gitlab
      git push gitlab master
  elif [ $REMOTE = $BASE ]; then
      echo "Need to push"
  else
      echo "Diverged"
  fi  
  
  echo
  sleep $DELAY

done
