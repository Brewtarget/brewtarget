#! /bin/bash -ex
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

$DIR/install-deps
source $DIR/common-vars

lookup_endpoint="https://api.github.com/repos/cgspeck/brewtarget/releases"
jq_query="first(.[] | select(.tag_name | contains(\"$TAG_NAME\")) | .id)"

release_id=$(curl -f -uChrisBuildAccount:$GITHUB_TOKEN $lookup_endpoint | jq "${jq_query}")

echo "Release ID is '${release_id}'"



patch_endpoint="https://api.github.com/repos/cgspeck/brewtarget/releases/${release_id}"
payload=$(cat <<EOF
{
    "draft": false
}
EOF
)

echo -e "\n\n Patching release with following payload: \n\n ${payload} \n\n"

curl -f -uChrisBuildAccount:$GITHUB_TOKEN \
    -XPATCH \
    -H "Content-Type: application/json" \
    -d "${payload}" \
    $patch_endpoint
