#!/bin/sh
#
# necro slack posts
# depends:
# 	* jq
# 	* curl
# 	* gnu/obsd date
# 	* cc (any tm)
#
# get channels with: get "/conversations.list" | jq '.channels[] | .id, .name'

progname="$0"

token="$1"
from="$2"
to="$3"

if [ -z "$token" -o -z "$from" -o -z "$to" ]; then
	echo "USAGE: $progname <token> <from> <to>" > /dev/stderr
	exit 1
fi

# Compat for passing in a timestamp to date(1). Tested with GNU and OpenBSD date
cdate() {
	ts="$1"
	shift
	(date --date="@$ts" "$@" || date --date="$ts" "$@" \
		|| date -r "$ts" "$@") 2>/dev/null
}

unquote() {
	sed 's/^"\(.*\)"$/\1/'
}

get() {
	url="https://slack.com/api$1"
	curl -s -H "Authorization: Bearer $token" -X GET "$url"
	echo
}

post() {
	url="https://slack.com/api$1"
	shift
	curl -s -X POST \
		-H "Authorization: Bearer $token" \
		-H 'Content-Type: application/json; charset=utf-8' \
		"$@" "$url"
	echo
}

dupeMessage() {
	ts="$1"
	msg="$(get "/conversations.history?channel=$from&latest=$ts&inclusive=true&limit=1" | jq '.messages[0]')"

	ts="$(echo "$ts" | cut -d'.' -f1)"
	recv="$(cdate $ts '+%Y-%m-%d %T')"

	user="$(echo "$msg" | jq '.user' | sed 's/"\(.*\)"/\1/')"
	sender="$(get "/users.info?user=$user" | jq '.user.real_name' | unquote)"

	text="$(echo "$msg" | jq '.text' | unquote)"
	attachments="$(echo "$msg" | jq '.files[].url_private' 2>/dev/null | unquote)"

	text="$(echo "($recv) $sender: $text $attachments")"
	test -z "$text" && text="slack lmao"

	echo "{\"channel\": \"$to\", \"text\": \"$text\" }"
}

post "/conversations.join" --data "{ \"channel\": \"$from\" }" > /dev/null
post "/conversations.join" --data "{ \"channel\": \"$to\" }" > /dev/null

test -f "89daysago" || cc -o 89daysago 89daysago.c
latest=$(./89daysago)
echo "Reviving messages before: $latest"

for ts in $(get "/conversations.history?channel=$from&latest=$latest" | jq '.messages[].ts' | unquote | sort); do
	echo "INFO: Reviving message from $ts"
	data="$(dupeMessage $ts)"
	post "/chat.postMessage" --data "$data"
	sleep 1 # https://api.slack.com/methods/chat.postMessage#rate_limiting
done
