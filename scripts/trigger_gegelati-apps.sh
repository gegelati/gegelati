#!/bin/bash
set -x

body='{"request": {"branch":"develop"}}'

curl -s -X POST -H "Content-Type: application/json" -H "Accept: application/json" -H "Travis-API-Version: 3" -H "Authorization: token ${TRAVIS_TOKEN}" -d "$body" https://api.travis-ci.com/repo/gegelati%2Fgegelati-apps/requests