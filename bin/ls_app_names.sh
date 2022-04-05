curl -sk http://localhost:8085/ls | jq '.appName' -Mrc | sort -u
