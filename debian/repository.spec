# Author: 2017-2018 Alf Gaida <agaida@siduction.org>
# License: WTFPL-2
#  0. You just DO WHAT THE FUCK YOU WANT TO.

# Variables
# =========
# baseversion= the first part of the version no (0.x.y~)
# distribution= target distribution
# repository= your repository
# branch= your branch, if needed
# commit= your commit , if needed
# target_dir= target dir, if needed, default is snapshot
#
# Functions
# =========
# cleanup ()

baseversion=3.3.1~
distribution=unstable
uploadrepo=extra

repository="git@github.com:KDE/kpmcore"
commit='b22be9ad9a39f'

build="yes"
clean="yes"
gitcommit="yes"
push="yes"
upload="yes"
dryrun="no"


# cleanup function definiton
cleanup() {
    echo ""
    echo "┌────────────────────┐"
    echo "│ Cleanup kpmcore... │"
    echo "└────────────────────┘"
    rm -rf .git*
    rm -f .arcconfig
}
