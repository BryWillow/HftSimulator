#!/bin/bash
# ---------------------------------------------------------------------------
# File        : git_commit_summary
# Project     : HftSimulator
# Description : Summarize staged git changes and generate a commit message draft
# Author      : Bryan Camp
# ---------------------------------------------------------------------------

set -euo pipefail

# Directories considered “components” for commit grouping
COMPONENTS=("src/common" "src/listener_app" "src/replayer_app" "src/strategies" "tests" "vendor" ".")

# Count added/removed lines for a file
summarize_file() {
    local file="$1"
    local added removed
    added=$(git diff --cached -- "$file" | grep -cE '^\+[^+]' || true)
    removed=$(git diff --cached -- "$file" | grep -cE '^-([^-]|$)' || true)
    printf " - %-50s (+%d / -%d)\n" "$file" "$added" "$removed"
}

# Determine component from file path
get_component() {
    local file="$1"
    for dir in "${COMPONENTS[@]}"; do
        if [[ "$file" == "$dir"* ]]; then
            echo "$dir"
            return
        fi
    done
    echo "other"
}

# Main summary
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "📌 Staged changes summary:"
    echo ""

    declare -A summary

    # Loop over staged files
    git diff --cached --name-only | while read -r file; do
        [[ ! -e "$file" ]] && continue
        summarize_file "$file"

        comp=$(get_component "$file")
        summary["$comp"]+=$(basename "$file")$'\n'
    done

    echo ""
    echo "📝 Suggested commit message draft:"
    echo ""

    for comp in "${COMPONENTS[@]}"; do
        if [[ -n "${summary[$comp]:-}" ]]; then
            prefix=""
            case "$comp" in
                src/common) prefix="feat(common): " ;;
                src/listener_app) prefix="refactor(listener): " ;;
                src/replayer_app) prefix="refactor(replayer): " ;;
                src/strategies) prefix="feat(strategy): " ;;
                tests) prefix="test: " ;;
                vendor) prefix="vendor: " ;;
                .) prefix="" ;;
                *) prefix="" ;;
            esac

            # First line = prefix + summary of files
            first_file=$(echo "${summary[$comp]}" | head -n1)
            echo "$prefix$first_file"

            # Optional: additional files
            other_files=$(echo "${summary[$comp]}" | tail -n +2)
            if [[ -n "$other_files" ]]; then
                while IFS= read -r f; do
                    echo "  - $f"
                done <<< "$other_files"
            fi

            echo ""
        fi
    done

else
    echo "Error: not inside a git repository"
    exit 1
fi
