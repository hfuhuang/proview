#!/bin/bash
#
# Proview   $Id: pwr_pkg.sh,v 1.5 2005-09-01 14:57:49 claes Exp $
# Copyright (C) 2005 SSAB Oxel�sund AB.
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as 
# published by the Free Software Foundation, either version 2 of 
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with the program, if not, write to the Free Software 
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

pkg_install_func ()
{
  pkg_remove_func "force"

  if [ $1 == ${1##*/} ]; then
    # Add path
    pkg="/home/pwrp/$1"
  else
    pkg=$1
  fi

  if [ ! -e "$pkg" ]; then
    echo "No package $pkg"
    exit 1
  fi

  echo "-- Installing package $1"
  cd /tmp
  if ! tar -xzf $pkg pkg_unpack.sh; then
    echo "Package $1 incomplete"
    exit 1
  fi

  chmod a+x pkg_unpack.sh
  ./pkg_unpack.sh
}

pkg_list_func ()
{

  if [ -z $1 ]; then
    # List installed package

    if [ ! -e $pwrp_load/pwr_pkg.dat ]; then
	echo "-- No package installed"
	exit 1
    fi  

    datfile=$pwrp_load/pwr_pkg.dat
  else
    # Extract datfile from package
    if [ $1 == ${1##*/} ]; then
      # Add path
      pkg="/home/pwrp/$1"
    else
      pkg=$1
    fi

    if [ ! -e $pkg ]; then
      echo "-- No such package"
      exit 1
    fi

    echo "-- Opening file $pkg"

    cd /tmp
    if ! tar -xzf $pkg pwr_pkg.dat; then
      echo "Package $1 incomplete"
      exit 1
    fi
    datfile=/tmp/pwr_pkg.dat
  fi

  {
    let printout=0
    while read line; do
      if [ "$line" = "%Description:" ]; then
        printout=1
      else
        if [ "$line" = "%Files:" ]; then
          break
        fi
        if [ $printout -eq 1 ]; then
          echo $line
        fi
      fi
    done
  } < $datfile
}

pkg_listfiles_func ()
{

  if [ -z $1 ]; then
    # List installed package

    if [ ! -e $pwrp_load/pwr_pkg.dat ]; then
	echo "-- No package installed"
	exit 1
    fi  

    datfile=$pwrp_load/pwr_pkg.dat
  else
    # Extract datfile from package
    if [ $1 == ${1##*/} ]; then
      # Add path
      pkg="/home/pwrp/$1"
    else
      pkg=$1
    fi

    if [ ! -e $pkg ]; then
      echo "-- No such package"
      exit 1
    fi

    echo "-- Opening file $pkg"

    cd /tmp
    if ! tar -xzf $pkg pwr_pkg.dat; then
      echo "Package $1 incomplete"
      exit 1
    fi
    datfile=/tmp/pwr_pkg.dat
  fi

  {
    let printout=0
    while read line date time; do
      if [ "$line" = "%Files:" ]; then
        printout=1
      else
        if [ "$line" = "%End:" ]; then
          break
        fi
        if [ $printout -eq 1 ]; then
          echo $date $time $line
        fi
      fi
    done
  } < $datfile
}

pkg_brief_func ()
{
  if [ -z $1 ]; then
    # List installed package

    if [ ! -e $pwrp_load/pwr_pkg.dat ]; then
	echo "-- No package installed"
	exit 1
    fi  

    datfile=$pwrp_load/pwr_pkg.dat
    {
      let printout=0
      while read line; do
        if [ "$line" = "%Brief:" ]; then
          printout=1
        else
          if [ "$line" = "%Description:" ]; then
            break
          fi
          if [ $printout -eq 1 ]; then
            echo $line
          fi
        fi
      done
    } < $datfile

  else
    # Extract datfile from package

    for file
    do
      if [ $file == ${file##*/} ]; then
        # Add path
        pkg="/home/pwrp/$file"
      else
        pkg=$file
      fi

      if [ ! -e $pkg ]; then
        echo "-- No such package"
        exit 1
      fi

      dir=`eval pwd`

      cd /tmp
      if ! tar -xzf $pkg pwr_pkg.dat; then
        echo "Package $pkg incomplete"
      else
        datfile=/tmp/pwr_pkg.dat

        {
          let printout=0
          while read line; do
            if [ "$line" = "%Brief:" ]; then
              printout=1
            else
              if [ "$line" = "%Description:" ]; then
                break
              fi
              if [ $printout -eq 1 ]; then
	        fname=`eval basename $pkg`
                echo $fname ${line#Proview package}
              fi
            fi
          done
        } < $datfile
      fi
      cd $dir
    done
  fi
}

pkg_dir_func()
{
  if [ -z $1 ]; then
    allpkg=`ls /home/pwrp/pwrp_pkg_*.tgz`
  else
    if [ $1 == ${1##*/} ]; then
      # Add path
      pattern="/home/pwrp/*$1*"
    elif [ ${1:0:1} == "/" ]; then
      pattern=$1*
    else
      pattern=*$1*
    fi
    allpkg=`ls $pattern`
  fi

  for pkg in $allpkg ; do
    if [ -z ${pkg##*pwrp_pkg_*.tgz} ]; then
      pkg_brief_func $pkg
    fi
  done
}

pkg_dirbrief_func()
{
  if [ -z $1 ]; then
    allpkg=`ls /home/pwrp/pwrp_pkg_*.tgz`
  else
    if [ $1 == ${1##*/} ]; then
      # Add path
      pattern="/home/pwrp/*$1*"
    elif [ ${1:0:1} == "/" ]; then
      pattern=$1*
    else
      pattern=*$1*
    fi
    allpkg=`ls $pattern`
  fi

  for pkg in $allpkg ; do
    if [ -z ${pkg##*pwrp_pkg_*.tgz} ]; then
      basename $pkg
    fi
  done
}

pkg_remove_func ()
{
  if [ ! -e $pwrp_load/pwr_pkg.dat ]; then
    echo "-- No package installed"
    return
  fi

  # Get the name of the current package
  {
    let found=0
    while read line; do
      if [ "$line" = "%Package:" ]; then
        found=1
      else
        if [ $found -eq 1 ]; then
	  pkg=$line
	  break
        fi
      fi
    done
  } < $pwrp_load/pwr_pkg.dat

  if [ ! "$1" = "force" ]; then
    echo ""
    echo -n "Do you wan't to remove package $pkg (y/n) [n] "
    read remove_pkg
    if [ ! "$remove_pkg" = "y" ]; then
      return
    fi
  fi

  echo "-- Removing package $pkg"

  {
    let removefile=0
    while read line date time; do
      if [ "$line" = "%Files:" ]; then
        removefile=1
      else
        if [ "$line" = "%End:" ]; then
          break
        fi
        if [ $removefile -eq 1 ]; then
	  file=`eval echo $line`
	  #echo "rm $file"
          rm $file
        fi
      fi
    done
  } < $pwrp_load/pwr_pkg.dat

  rm $pwrp_load/pwr_pkg.dat
}

force="no"
while [ -n "$(echo $1 | grep '-')" ]; do

  OPTARG=$2
  case $1 in
    -i ) pkg_install_func $OPTARG ;;

    -la ) pkg_list_func $OPTARG ;;

    -lf ) pkg_listfiles_func $OPTARG;;

    -lp ) shift
        pkg_brief_func $@ ;;

    -r ) pkg_remove_func ;;

    -rf ) force="force" 
          pkg_remove_func $force ;;

    -ld ) pkg_dir_func $OPTARG ;;

    -l ) pkg_dirbrief_func $OPTARG;;

    * ) cat <<EOF 
    usage: pwr_pkg [-i pkg] [-l [pkg]] [-b [pkg]] [-r]

    pwr_pkg -i 'pkg'      Install package 'pkg'
    pwr_pkg -r            Remove currently installed package
    pwr_pkg -lp ['pkg']   List installed package, or package 'pkg'
    pwr_pkg -la ['pkg']   List installed package, or package 'pkg', all info
    pwr_pkg -lf ['pkg']   List files in installed package, or package 'pkg'
    pwr_pkg -l            List all packages

EOF
         exit 1
  esac
  shift
done








