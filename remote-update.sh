#!/bin/bash

echo "Bind to the remote repo"
bzr bind
echo "Updating..."
bzr update
echo "Unbind from the remote repo"
bzr unbind
