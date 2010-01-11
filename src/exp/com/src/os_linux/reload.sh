#!/bin/bash
#
# Proview   $Id: reload.sh,v 1.11 2008-04-10 10:38:30 claes Exp $
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
#
#
#
#

let reload__success=0
let reload__userclasses=1
let reload__usertypes=2
let reload__settemplate=3
let reload__loaddb=4

let pass__continue=1
let pass__execute=2

reload_dblist_read()
{
  local volume
  local volid
  local volclass
  local volcnf
  local volenum
  local volserver
  local dbfile
  local db
  let i=0
  let j=0
  let k=0

  pwrc_status=$pwrc__success

  if [ -n "${db_array[*]}" ]; then
    unset db_array
  fi

  dbfile=$pwrp_root/src/db/pwrp_cnf_volumelist.dat
  if [ ! -e $dbfile ]; then
    echo "Can't find $dbfile"
    return
  fi

  while read volume volid volclass volcnf volenum volserver; do
    if [ -n "$volume" ] && [ "${volume:0:1}" != "!" ]; then      
      if [ $volcnf == "cnf" ]; then
        if [ $volclass == "ClassVolume" ]; then
          if [ $volenum -eq 0 ]; then
            wbl_array[$k]=`eval echo $volume | tr "[:upper:]" "[:lower:]"`
            k=$k+1            
          elif [ $volenum -eq 1 ]; then
            db_array[$i]=`eval echo $volume | tr "[:upper:]" "[:lower:]"`
            i=$i+1
          elif [ $volenum -eq 2 ]; then
            dbms_array[$j]=`eval echo $volume | tr "[:upper:]" "[:lower:]"`
            dbms_server_array[$j]=$volserver
            j=$j+1
          fi
        else
          if [ $volenum -eq 0 ]; then
            db_array[$i]=`eval echo $volume | tr "[:upper:]" "[:lower:]"`
            i=$i+1
          elif [ $volenum -eq 1 ]; then
            dbms_array[$j]=`eval echo $volume | tr "[:upper:]" "[:lower:]"`
            dbms_server_array[$j]=$volserver
            j=$j+1
          fi
        fi
      fi
    fi
  done < $dbfile

}

reload_restoredb()
{
  echo file $1
  if [ -z $pwrp_db/$1 ]; then
    echo "** Database backup not found"
    reload_status=$reload__success
    return
  fi

  db=${1%.[1-9]}

  reload_dblist_read

  if [ "${db#[a-z]*.}" == "dbms" ]; then
    echo "Restore mysql database $db"	     

    found=0
    idx=0
    i=0
    while [ "${dbms_array[$i]}" != "" ]; do
      if [ "${dbms_array[$i]}" == "${db%.[a-z]*}" ]; then
        found=1
        idx=$i
        break
      fi
      i=$i+1
    done

    if [ $found -eq 0 ]; then
      echo "** Volume ${db%.[a-z]*} is not configured"
      reload_exit
    fi

    if [ -e $pwrp_db/$db ]; then
      # Delete the current database

      echo -n "Do you want to delete $db ? [y/n] "
      read repl
      if [ "$repl" != "y" ]; then
        reload_exit
      fi

      echo "-- Deleting ${dbms_array[$idx]} on server ${dbms_server_array[$idx]}"

      dbname="pwrp_"$pwrp_projectname"__"${dbms_array[$idx]}
      mysqladmin -h ${dbms_server_array[$idx]} -upwrp drop -f $dbname

      rm -r $pwrp_db/$db
    fi

    dbname="pwrp_"$pwrp_projectname"__"${dbms_array[$idx]}

    if [ ! -e $pwrp_db/$1/$dbname.mysqldump ]; then
      echo "** No mysql dump found, $pwrp_db/$1/$dbname.mysqldump"
      reload_exit
    fi

    mv $pwrp_db/$1 $pwrp_db/$db
    mysqladmin -h ${dbms_server_array[$idx]} -upwrp create $dbname
    cat $pwrp_db/$db/$dbname.mysqldump | mysql -h ${dbms_server_array[$idx]} -upwrp -D $dbname

  elif [ "${db#[a-z]*.}" == "db" ]; then
    echo "Restore BerkeleyDb database $db"

  fi
}

reload_classvolumes()
{
  reload_checkpass "classvolumes" $start_pass

  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  list=`eval ls -1d $pwrp_db/*.wb_load`
  echo ""
  for file in $list; do
    volume=`eval grep pwr_eClass_ClassVolume $file | awk '{ print $2 }'`
    if [ "$volume" == "" ]; then
      volume=`eval grep ClassVolume $file | awk '{ print $2 }'`
      volumelow=`eval grep ClassVolume $file | awk '{ print tolower($2) }'`
    fi
    if [ "$volume" != "" ]; then
      echo $file
    fi
  done
  echo ""

  reload_continue "Pass create structfiles and loadfiles for classvolumes"

  list=`eval ls -1d $pwrp_db/*.wb_load`
  for file in $list; do
    if [ ${file##/*/} != "directory.wb_load" ]; then
      volume=`eval grep pwr_eClass_ClassVolume $file | awk '{ print $2 }'`
      volumelow=`eval grep pwr_eClass_ClassVolume $file | awk '{ print tolower($2) }'`
      if [ "$volume" == "" ]; then
        volume=`eval grep ClassVolume $file | awk '{ print $2 }'`
        volumelow=`eval grep ClassVolume $file | awk '{ print tolower($2) }'`
      fi
      if [ "$volume" != "" ]; then
        echo "-- Creating structfile and loadfile for $volume"
        if co_convert -sv -d $pwrp_inc $file
        then
          reload_status=$reload__success
        else
          reload_status=$reload__userclasses
          return
        fi

        if wb_cmd -q create snapshot/file=\"$file\"/out=\"$pwrp_load/$volumelow.dbs\"
        then
          reload_status=$reload__success
        else
          reload_status=$reload__userclasses
          return
        fi
      fi
    fi
  done
}

reload_dumpdb()
{
  reload_checkpass "dumpdb" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass dump database"

  # Rename old dumpfiles
  dmpfiles=`eval ls -1 $pwrp_db/*.wb_dmp 2> /dev/null`
  for dmpfile in $dmpfiles; do
    reload_save_file $dmpfile
  done

  if [ $arg_found -eq 1 ]; then
    # Dump supplied database
    if [ $arg_db -eq 1 ]; then
      cdb=${db_array[$arg_idx]}
    else
      cdb=${dbms_array[$arg_idx]}
    fi
    dump_file=$pwrp_db/$cdb.wb_dmp
    echo "-- Dumping volume $cdb into $dump_file"
    wb_cmd -q -v $cdb wb dump/nofocode/out=\"$dump_file\"
  else
    # Dump all databases
    i=0
    while [ "${db_array[$i]}" != "" ]; do
      dump_file=$pwrp_db/${db_array[$i]}.wb_dmp
      echo "-- Dumping volume ${db_array[$i]} into $dump_file"
      wb_cmd -q -v ${db_array[$i]} wb dump/nofocode/out=\"$dump_file\"
      i=$i+1
    done
    i=0
    while [ "${dbms_array[$i]}" != "" ]; do
      dump_file=$pwrp_db/${dbms_array[$i]}.wb_dmp
      echo "-- Dumping volume ${dbms_array[$i]} into $dump_file"
      wb_cmd -q -v ${dbms_array[$i]} wb dump/nofocode/out=\"$dump_file\"
      i=$i+1
    done
  fi
}

reload_renamedb()
{
  reload_checkpass "renamedb" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass rename old database"


  if [ $arg_found -eq 1 ]; then
    # Rename supplied database
    if [ $arg_db -eq 1 ]; then
      reload_save_file $pwrp_db/${db_array[$arg_idx]}.db
    else
      dbname="pwrp_"$pwrp_projectname"__"${dbms_array[$arg_idx]}
      mysqldump -h ${dbms_server_array[$arg_idx]} -upwrp $dbname > $pwrp_db/${dbms_array[$arg_idx]}.dbms/$dbname.mysqldump

      mysqladmin -h ${dbms_server_array[$arg_idx]} -upwrp drop -f $dbname

      reload_save_file $pwrp_db/${dbms_array[$arg_idx]}.dbms
    fi
  else
    # Rename all databases
    i=0
    while [ "${db_array[$i]}" != "" ]; do
      reload_save_file $pwrp_db/${db_array[$i]}.db
      i=$i+1
    done
    i=0
    while [ "${dbms_array[$i]}" != "" ]; do
      dbname="pwrp_"$pwrp_projectname"__"${dbms_array[$i]}
      mysqldump -h ${dbms_server_array[$i]} -upwrp $dbname > $pwrp_db/${dbms_array[$i]}.dbms/$dbname.mysqldump

      mysqladmin -h ${dbms_server_array[$i]} -upwrp drop -f $dbname

      reload_save_file $pwrp_db/${dbms_array[$i]}.dbms
      i=$i+1
    done
  fi
 
}

reload_dirvolume()
{
  if [ -e "$pwrp_db/directory.db" ]; then
    return
  fi

  reload_checkpass "dirvolume" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass dirvolume"

  wb_cmd -q create volume/directory
  wb_cmd -q wb load /load=\"$pwrp_db/directory.wb_dmp\"
}

reload_loaddb()
{
  reload_checkpass "loaddb" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass load database"

  for cdb in $databases; do
    echo "-- Loading volume $cdb"
     
    dump_file=$pwrp_db/$cdb.wb_dmp
    list_file=$pwrp_db/$cdb.lis
    if wb_cmd -q wb load/nofocode/load=\"$dump_file\"/out=\"$list_file\"
    then
      reload_status=$reload__success
    else
      cat $list_file
      reload_status=$reload__loaddb
    fi
  done
}

reload_compile()
{
  reload_checkpass "compile" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass compile plcprograms"

  for cdb in $databases; do
    wb_cmd -q -v $cdb compile /all
  done

  reload_status=$reload__success
}

reload_removeload()
{
  reload_checkpass "removeload" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass remove old loadfiles"

  reload_status=$reload__success
}

reload_createload()
{
  reload_checkpass "createload" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass create loadfiles"

  for cdb in $databases; do
    echo "-- Creating loadfiles for database $cdb"
    wb_cmd -q -v $cdb create load
  done
  reload_status=$reload__success
}

reload_createboot()
{
  reload_checkpass "createboot" $start_pass
  if [ $pass_status -ne $pass__execute ]; then
    reload_status=$reload__success
    return
  fi

  reload_continue "Pass create bootfiles"

  echo "-- Creating bootfiles for all nodes"
  wb_cmd -q create boot/all
  reload_status=$reload__success
}

reload_exit()
{
  exit $reload_status
}

reload_continue()
{
  echo
  echo "----------------------------------------------------------------------------------------"
  echo " $1"
  echo "----------------------------------------------------------------------------------------"
  if [ $go -eq 1 ]; then
    return
  fi

  echo -n "Do you want to continue ? [y/n/go] "
  read repl
  case $repl in
    go ) go=1; return ;;
    y ) return ;;
    n ) reload_exit ;;
  esac

  reload_continue "$1"
}

reload_checkpass()
{
  checkpass=$1
  wantedpass=$2

  pass_status=$pass__continue
  for item in $passes; do
    if [ $item = $wantedpass ]; then
      pass_status=$pass__execute
    fi
    if [ $item = $checkpass ]; then
      return
    fi
  done

  echo "No such pass"
  reload_exit
}

reload_save_file()
{
  new_file=$1
  
  if [ -e $new_file ]; then
    let version=9

    while [ $version -ge 1 ]
    do
      old_file=$new_file.$version
      old_file_ren=$new_file.$((version+1))
      if [ -e $old_file ]; then
        mv $old_file $old_file_ren
      fi
      let version=$version-1
    done

    old_file=$new_file.1
    echo "-- Saving file $new_file -> $old_file"
    mv $new_file $old_file
  fi
}

usage()
{
  cat << EOF

  reload.sh   Dump and reload of database.

  Arguments   Database or databases to reload.
              I no arguments is supplied, all databases will be reloaded.

  Pass

    dumpdb       Dump database to textfile \$pwrp_db/'volume'.wb_dmp
    classvolumes Create structfiles and loadfiles for classvolumes
    renamedb     Rename the old database
    loaddb       Load the dump into the new database
    compile      Compile all plcprograms in the database
    createload   Create new loadfiles.
    createboot   Create bootfiles for all nodes in the project.

EOF
}

if [ "$1" = "help" ] || [ "$1" = "-h" ]; then
  usage
  exit
fi


let reload_status=$reload__success
let check_status=0
let go=0
let arg_found=0
let arg_dbms=0
let arg_db=0
let arg_wbl=0
let arg_idx=0

reload_dblist_read

if [ "$1" == "restore" ]; then
  if [ $# -ne 2 ]; then
    echo "Invalid arguments"
    reload_exit
  fi
  shift
  reload_restoredb $1
  reload_exit
fi

if [ "$1" != "" ]; then
  i=0
  while [ "${db_array[$i]}" != "" ]; do
    if [ $1 == ${db_array[$i]} ]; then
      arg_found=1
      arg_idx=i
      arg_db=1
      databases=${db_array[$i]}
      break
    fi
    i=$i+1
  done
  i=0
  while [ "${dbms_array[$i]}" != "" ]; do
    if [ $1 == ${dbms_array[$i]} ]; then
      arg_found=1
      arg_idx=i
      arg_dbms=1
      databases=${dbms_array[$i]}
      break
    fi
    i=$i+1
  done
  i=0
  while [ "${wbl_array[$i]}" != "" ]; do
    if [ $1 == ${wbl_array[$i]} ]; then
      arg_found=1
      arg_idx=1
      arg_wbl=1
      break
    fi
    i=$i+1
  done

  if [ $arg_found -eq 0 ]; then
    echo "** No such database"
    reload_exit
  fi
  if [ $arg_wbl -eq 1 ]; then
    echo "** Database has wb_load format, no reload possible"
    reload_exit
  fi
else
  i=0
  databases=""
  while [ "${db_array[$i]}" != "" ]; do
    databases="$databases ${db_array[$i]}"
    i=$i+1
  done
  i=0
  while [ "${dbms_array[$i]}" != "" ]; do
    databases="$databases ${dbms_array[$i]}"
    i=$i+1
  done
fi

usage

echo ""
echo "-- Reloading volume $databases"
echo ""

passes="dumpdb classvolumes renamedb loaddb compile createload createboot"
echo "Pass: $passes"
echo ""
echo -n "Enter start pass [dumpdb] > "
read start_pass

if [ -z $start_pass ]; then
  start_pass="dumpdb"
fi

for cpass in $passes; do
  reload_$cpass
  if [ $reload_status -ne $reload__success ]; then
    echo "Exiting due to error"
    reload_exit
  fi
done

echo ""
echo "-- The reload is now accomplished."
echo "   Please remove the dumpfiles: \$pwrp_db/*.wb_dmp*"
echo "   when you are satisfied with the reload."
echo ""

reload_exit











