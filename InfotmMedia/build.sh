#!/bin/bash
###  sam_ye, 2012/06/02  ###

### to fix the current path
LOCAL_PATH=`pwd`
cd ..
TMP_PATH=`pwd`
TMP_PATH=${TMP_PATH/%\/}
cd $LOCAL_PATH
if [ "$TMP_PATH/" = "${LOCAL_PATH/%InfotmMedia}" ]
then
	cd ${0/build.sh}
fi
BUILD_ROOT=`pwd`

state_file="env_state_must_not_delete"
config_file="./config.mk"
IM_EXTERNAL_ROOT=$BUILD_ROOT/external/project/
SYS_ROOT="../../../../../../../../../"
WP_PATH="../../workcopy/"
ext_file_list="change_file_list.txt"

###########################################################

### error print 
print_err()
{
	echo "error: "$1" failed "$2"!"
}

copy_file()
{
	cp $1 $2
	if [ $? -ne 0 ] ; then
		print_err "copy file" $1
		exit 1
	fi
}

###  user help
print_help()
{
	echo "####################################################################################"
	echo "$0 [help/module/external_module]"
	echo "	builder the indicated module envirionment, if not set, modules in config.mk will build."
	echo "	module: optional, module folder."
	echo "	external_module: optionanl, external module folder."
	echo "example:"
	echo "	$0"
	echo "	$0 foundations"
	echo "	$0 external_hwcursor"
	echo "###################################################################################"
}

### check the module state : $1 = "$dir" , $2 = "$str"
check_state()
{
	dir=$1
	str=$2
	if [ $# -ne 2 ]; then
		echo "error: check state parameters error!" 
		exit 1
	fi

	if [ ! -f $state_file ]; then 
		touch $state_file
		echo "### stores the module enviroment state for scripts to check ###" >> $state_file
		echo "###  Do not modify it ###" >> $state_file
		echo "" >> $state_file
	fi
	
	line=`grep $str $state_file`
	if [ "$line" = "" ]; then
		echo $str" = 0" >> $state_file
		curr_state=0
	else 
		curr_state=`echo $line | awk '{print $NF}'`
	fi
	if [ $curr_state -eq 1 ] 
	then
		echo "warn: $dir has already built, would't do it more than once."
		continue
	fi
	return $curr_state
}

### compiler module enviroment build
comp_build()
{
	exclude=".svn"

	for dir in `ls -l | grep ^d | awk '{print $NF}'`
	do
		if [ $1 = "all" ] || [ $dir = $1 ]; then
			### check the state
			if [ ! -f $state_file ] ; then 
				touch $state_file
				echo "### stores the module enviroment state for scripts to check ###" >> $state_file
				echo "###  Do not modify it ###" >> $state_file
				echo "" >> $state_file
			fi
			str="IM_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo $str" = 0" >> $state_file
				curr_state=0
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi

			
			if [ $curr_state -eq 1 ] ; then
				echo "warn: $dir has already built, would't do it more than once."
				continue
			fi
			
			## execute the script if needed.
			cd $dir
			if [ -f buildenv.sh ] ; then
				./buildenv.sh 
				if [ $? -ne 0 ]; then
					print_err $dir "buildenv"
				fi
			fi
			cd - 1>/dev/null
			sed -i "s/$str = 0/$str = 1/g" $state_file
		fi
	done
}

###  real external module 
ext_change_file_list_build()
{
	if [ ! -f $ext_file_list ]; then
		return 0
	fi

	exec 5<> $ext_file_list
	cnt=0
	while read line <&5
	do {
		((cnt ++))
		len=`echo $line | awk '{print length()}'`
		if [ $len -eq 0 ] || [ "$line" != "${line/"#"}" ];then
			continue
		else
			file=$line
		fi
		
		if [ -f $DRV_ROOT/$file ]; then
			copy_file $DRV_ROOT/$file $WP_PATH/$file.tmpbak
		fi
		copy_file $WP_PATH/$file $DRV_ROOT/$file
	}
	done
	exec 5>&-

}

### external module enviroment build
### should check the state first, then call the truely dealing function
ext_build()
{
	for dir in `ls -l $IM_EXTERNAL_ROOT | grep ^d | awk '{print $NF}'`
	do
		if [ $1 = "all" ] || [ $dir = $1 ]; then
			if [ ! -f $state_file ] ; then 
				touch $state_file
				echo "### stores the module enviroment state for scripts to check ###" >> $state_file
				echo "###  Do not modify it ###" >> $state_file
				echo "" >> $state_file
			fi
			str="IM_EXTERNAL_"$dir"_state"
			line=`grep $str $state_file`
			if [ "$line" = "" ]; then
				echo $str" = 0" >> $state_file
				curr_state=0
			else 
				curr_state=`echo $line | awk '{print $NF}'`
			fi

			if [ $curr_state -eq 1 ] ; then
				echo "warn: ${dir} has already built, would't do it more than once."
				continue
			fi

			cd $IM_EXTERNAL_ROOT/$dir

			if [ -f buildenv.sh ]
			then
				echo "execute " $dir "buildenv.sh"
				./buildenv.sh
				if [ $? -ne 0 ]
				then
					print_err "external buildenv.sh" $dir
					exit 1
				fi
			fi

			echo " external build: " $dir
		   	ext_change_file_list_build	
			cd - 1>/dev/null
			sed -i "s/$str = 0/$str = 1/g" $state_file
		fi
	done
}

build_env()
{
	exec 103<> $config_file
	cnt=0
	while read line <&103
	do {
		((cnt++))

		len=`echo $line | awk '{print length()}'`

		if [ $len -ne 0 ] && [ "$line" = "${line/"#"}" ]
		then
			COM_index=`echo $line | awk '{print index($line, "IM_SUPPORT_")}'`
			EXT_index=`echo $line | awk '{print index($line, "IM_SUPPORT_EXTERNAL_")}'`
			if [ $COM_index -eq 1 ] && [ $EXT_index -eq 0 ] 
			then
				dir=`echo $line | awk '{a=12; b=index($line, ":="); print substr($line,a,b-a)}'`
				yes_no=`echo $line | awk '{a=index($line,":="); print substr($line,a+3,4)}'`
				if [ $yes_no = "true" ] 
				then
					comp_build $dir
				fi

			#elif [ $EXT_index -eq 1 ]
			#then
			#	dir=`echo $line | awk '{a=21; b=index($line, ":="); print substr($line,a,b-a)}'`
			#	yes_no=`echo $line | awk '{a=index($line,":="); print substr($line,a+3,4)}'`
			#	if [ $yes_no = "true" ]
			#	then
			#		ext_build $dir
			#	fi

			fi
		fi

	}
	done
	exec 103>&-
}

############################################################

if [ $# -eq 1 ] && [ $1 = "help" ] ; then 
	print_help
	exit 0
fi

## no params, build all the enviroment
if [ $# -eq 0 ] ; then 
	echo
	build_env
	echo "##########build system enviroment success##########"
	echo 
	exit 0
fi

exit 0


