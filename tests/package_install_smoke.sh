#!/usr/bin/env sh
set -eu

source_dir="$1"
build_dir="$2"

install_dir="/tmp/skill-rating-cpp-install-smoke"
consumer_build_dir="/tmp/skill-rating-cpp-consumer-smoke"

rm -rf "$install_dir" "$consumer_build_dir"

cmake --install "$build_dir" --prefix "$install_dir" >/tmp/skill_rating_install_smoke.log
cmake -S "$source_dir/tests/package_consumer" -B "$consumer_build_dir" -DCMAKE_PREFIX_PATH="$install_dir" >/tmp/skill_rating_consumer_configure.log
cmake --build "$consumer_build_dir" >/tmp/skill_rating_consumer_build.log
"$consumer_build_dir/package_consumer"
