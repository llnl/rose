#!/bin/sh -e

######
#
# This script insert a new version of EDG into ROSE's EDG submodule.
# It patches this new version based on the current version of EDG used by ROSE.
# It is independent of the directory it is launch in.
#
# Params:
#  $1 - path to EDG submodule
#  $2 - path to EDG releases
#  $3 - current version
#  $4 - new version
#
######

set -e

# TODO check args
edg_submodule=$1
edg_release=$2
version_current=$3
version_new=$4

rose_edg_current=$edg_submodule/EDG_$version_current
rose_edg_new=$edg_submodule/EDG_$version_new
release_current=$edg_release/release_$version_current
release_new=$edg_release/release_$version_new

# TODO check dirs exist

for p in $(find $release_current -name *.c); do mv $p $p\c; done
for p in $(find $release_new -name *.c); do mv $p $p\c; done


diff_dir=edg_diff_$3_$4

rm -rf $diff_dir
mkdir -p $diff_dir/src $diff_dir/misc

export DIFF="diff --context=10"

echo "***********************"
echo "Creating patches..."

# Create patches for EDG/src/
for file_name in $(ls -l $release_current/src/ | grep -v "^d" | grep -v "Makefile" | awk '{print $9}'); do

  release_current_file=$release_current/src/$file_name
  rose_current_file=$(find $rose_edg_current/src -maxdepth 1 -name "$file_name")

  if [ ! -z "$rose_current_file" ]; then
    ($DIFF $release_current_file $rose_current_file > $diff_dir/src/$file_name.diff) || true
  fi
done

# Remove empty patches
for file_name in $(ls $diff_dir/src); do
  if [ ! -s $diff_dir/src/$file_name  ]; then
    rm $diff_dir/src/$file_name 
  fi
done

# Create patches for EDG/misc/
for file_name in $(ls -l $release_current/misc/ | grep -v "^d" | grep -v "Makefile" | awk '{print $9}'); do
  release_current_file=$release_current/misc/$file_name
  rose_current_file=`find $rose_edg_current/misc -maxdepth 1 -name "$file_name"`

  if [ ! -z "$rose_current_file" ]; then
    ($DIFF $release_current_file $rose_current_file > $diff_dir/misc/$file_name.diff) || true
  fi
done

# Remove empty patches
for file_name in $(ls $diff_dir/misc/); do
  if [ ! -s $diff_dir/misc/$file_name ]; then
    rm $diff_dir/misc/$file_name
  fi
done

echo "***********************"
echo "Create new EDG directory in ROSE' submodule..."

rm -rf $rose_edg_new

# Copy EDG release in ROSE's submodule
cp -r $release_new $rose_edg_new
chmod -R +w $rose_edg_new

# Rename EDG's original makefiles
find $rose_edg_new -name "Makefile" -exec bash -c 'mv $0 $0.edg ' {} \;

# Copy over ROSE's makefile for EDG
cp $rose_edg_current/Makefile.am $rose_edg_new/
cp $rose_edg_current/misc/Makefile.am $rose_edg_new/misc/
cp $rose_edg_current/src/Makefile.am $rose_edg_new/src/
cp $rose_edg_current/src/disp/Makefile.am $rose_edg_new/src/disp/
cp $rose_edg_current/lib/Makefile.am $rose_edg_new/lib/
cp $rose_edg_current/lib/Makefile.am $rose_edg_new/lib/

# Copy scripts used by ROSE when building EDG
cp $rose_edg_current/misc/lldef.sh $rose_edg_new/misc/
#cp $rose_edg_current/misc/am-config.sh $rose_edg_new/misc/ # not used in EDG 5.0
cp $rose_edg_current/misc/dettarg.out.sh $rose_edg_new/misc/

# Copy and update ROSE's CMake files
cp $rose_edg_current/misc/concat.cmake $rose_edg_new/misc/
cp $rose_edg_current/misc/generate_dettarg.out.cmake $rose_edg_new/misc/
cp $rose_edg_current/CMakeLists.txt $rose_edg_new/
cp $rose_edg_current/src/disp/CMakeLists.txt $rose_edg_new/src/disp/
cat $rose_edg_current/src/CMakeLists.txt | sed 's/EDG_${version_current}/EDG_${version_new}/g' > $rose_edg_new/src/CMakeLists.txt
cat $rose_edg_current/misc/CMakeLists.txt | sed 's/EDG_${version_current}/EDG_${version_new}/g' > $rose_edg_new/misc/CMakeLists.txt

# Copy define.h.rose
cp $rose_edg_current/src/defines.h.rose $rose_edg_new/src/

pushd $edg_submodule
git add EDG_$version_new
git commit -m "Added EDG $version_new source"
popd

echo "***********************"
echo "Apply patches..."

# Apply patches in EDG/src/
for file_name in $(ls $diff_dir/src/*.diff | awk 'BEGIN{FS="/"}{print $NF}' | awk 'BEGIN{FS="."}{printf "%s.%s\n", $1, $2}'); do
  patch $rose_edg_new/src/$file_name < $diff_dir/src/$file_name.diff || true
done

# Apply patches in EDG/src/
for file_name in $(ls $diff_dir/misc/*.diff | awk 'BEGIN{FS="/"}{print $NF}' | awk 'BEGIN{FS="."}{printf "%s.%s\n", $1, $2}'); do
  patch $rose_edg_new/misc/$file_name < $diff_dir/misc/$file_name.diff || true
done

pushd $edg_submodule
git add EDG_$version_new
git commit -m "Patched EDG $version_new source"
popd

echo "***********************"

echo "List of rejected hunks:"
find $rose_edg_new -name "*.rej"

echo "***********************"

echo "TODO list:"
echo " - applied rejected hunks"
echo " - remove any \"rm $rose_edg_new/src/*.orig $rose_edg_new/src/*.rej\""
echo " - add SUBDIRS, EXTRA_DIST, and rules in $edg_submodule/Makefile.am"
echo " - create ROSE_USE_EDG_VERSION_$version_new in AM_CONDITIONAL in ROSE/config/support-edg.m4"
echo " - add $rose_edg_new in EDG specific AM_CONFIG_OUTPUT in ROSE/config/support-rose.m4"
echo " - modify ROSE/build script to create dummy automake output (when EDG source is not present), otherwise autoconf will fail"
echo "Then you can build/compile/test..."

echo "***********************"

