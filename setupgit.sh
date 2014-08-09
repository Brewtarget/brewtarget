#!/bin/bash

echo ""
echo "Setting up brewtarget git preferences"
echo ""

# Enforce indentation with spaces, not tabs.
git config --file .git/config core.whitespace tabwidth=3,tab-in-indent

# Enable the pre-commit hook that warns you about whitespace errors
cp .git/hooks/pre-commit.sample .git/hooks/pre-commit
