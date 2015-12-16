#!/bin/bash

echo "Unbind from the repo"
bzr unbind
echo "local commit"
bzr commit
echo "Bind to the remote repo"
bzr bind
echo "Updating..."
bzr update
echo "remote commit"
bzr commit
echo "Unbind from the remote repo"
bzr unbind
