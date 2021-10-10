#!/bin/bash
SRC_DIRS=("scripts" "include" "src" "test" "docs")
IGNORE_DIRS=("node_modules\/" "build\/" "book\/")
SRC_FILE_PATTERNS=("\.c" "\.cpp" ".h" "CMakeLists\.txt" "\.sh" "\.md")
IGNORE_FILE_PATTERNS=("\.json")

SFILES=0
SLOC=0

function compile_regex_patterns() {
 if [[ -z "$@" || $# -eq 0 ]]; then
  printf 'compile_regex_patterns requires arguments!\n'
  exit -1
 fi
 local compiled_pattern=""
 for ((i=0; i < $#; i++)); do
  local index=$(($i + 1))
  if [[ $i == 0 ]]; then
   compiled_pattern="${!index}"
  else
   compiled_pattern="$compiled_pattern\|${!index}"
  fi
 done
 echo "$compiled_pattern"
}

function get_source_files() {
 if [[ -z "$1" ]]; then
  printf 'get_source_files requires a directory!\n'
  exit -1
 fi
 #Get all files
 local files="$(find $1 -type f)"
 #Remove ignored directories
 for ignore in "${IGNORE_DIRS[@]}"; do
  files=$(echo "$files" | grep -vi "$ignore")
 done
 #Remove files not matching any source patterns
 local sources_pattern="$(compile_regex_patterns ${SRC_FILE_PATTERNS[@]})"
 local ignore_pattern="$(compile_regex_patterns ${IGNORE_FILE_PATTERNS[@]})"
 files=$(echo "$files" | grep -i "$sources_pattern" | grep -vi "$ignore_pattern")
 echo "$files"
}

printf 'Matching patterns: %s\n' "$(compile_regex_patterns ${SRC_FILE_PATTERNS[@]})"
printf 'Ignore patterns: %s\n' "$(compile_regex_patterns ${IGNORE_FILE_PATTERNS[@]})"
for dir in "${SRC_DIRS[@]}"; do
 printf 'Scanning directory: %s\n' "$dir"
 source_files="$(get_source_files $dir)"
 #Convert newline delimited string into array
 OLD_IFS=$IFS
 IFS=$'\n'
 sources=($source_files)
 DIR_SLOC=0
 DIR_SFILES=0
 for file in ${sources[@]}; do
  if [[ ! -z "$file" ]]; then
   LOC=$(wc -l "${file}" | awk '{ print $1 }')
   DIR_SLOC=$(($DIR_SLOC + $LOC))
   SLOC=$(($SLOC + $LOC))
   SFILES=$(($SFILES + 1))
   DIR_SFILES=$(($DIR_SFILES + 1))
  fi
 done
 IFS=$OLD_IFS
 printf '\tSFILES: %s\n\tSLOC: %s\n' "$DIR_SFILES" "$DIR_SLOC"
done

printf 'SFILES: %s\nSLOC: %s\n' "$SFILES" "$SLOC"
