#!/bin/sh
set -e

chapter="$1"
if [ -z "$1" ]; then
	echo "USAGE: $0 <chapter-id>\n" > /dev/stderr
	exit 1
fi

chapterinfo=$(curl -s "https://api.mangadex.org/at-home/server/$chapter")
baseurl=$(echo $chapterinfo | jq -r .baseUrl)
chapterhash=$(echo $chapterinfo | jq -r .chapter.hash)

for file in $(echo $chapterinfo | jq -r .chapter.data[]); do
	echo "Saving $file..."
	curl -Os "$baseurl/data/$chapterhash/$file"
done

