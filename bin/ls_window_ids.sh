#!/usr/bin/env bash
curl -sk http://localhost:8085/ls| jq '.windowId' -Mrc | sort -u
