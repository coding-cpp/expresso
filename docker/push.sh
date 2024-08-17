#!/bin/bash

set -e

echo "Fetching version..."
VERSION_FILE="./include/expresso/version.h"
MAJOR_VERSION=$(grep -oP '(?<=#define EXPRESSO_VERSION_MAJOR )\d+' "$VERSION_FILE")
MINOR_VERSION=$(grep -oP '(?<=#define EXPRESSO_VERSION_MINOR )\d+' "$VERSION_FILE")
PATCH_VERSION=$(grep -oP '(?<=#define EXPRESSO_VERSION_PATCH )\d+' "$VERSION_FILE")
DOCKER_TAG="$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION"
echo "Current version: $DOCKER_TAG"

echo ""
echo "--------------------"
echo ""

echo "Building and pushing Docker image..."
docker buildx build -f docker/build.dockerfile --platform linux/amd64,linux/arm/v7 -t jadit19/expresso:latest -t jadit19/expresso:$DOCKER_TAG --push .
echo "Done!"