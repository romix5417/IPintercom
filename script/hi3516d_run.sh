#!/bin/sh

NETWORK_CFG="/etc/init.d/S80network"

FIRST_FLAG="yes"

TMPFILE_LINPHONE_CFG="/tmp/linphonec.conf"
TMPFILE_IPINTERCOM_CFG="/tmp/IPinterCom.cfg"
TMPFILE_RTSPSERVER_CFG="/tmp/rtspServer.cfg"
TMPFILE_FIRMWARE_CFG="/tmp/firmware.info"

HI3516_LINPHONE_CFG="/etc/config/linphonerc_orig"
HI3516_LINPHONE_RUN_CFG="/etc/config/linphonerc"
HI3516_IPINTERCOM_CFG="/etc/config/IPinterCom.cfg"
HI3516_RTSPSERVER_CFG="/etc/config/rtspServer.cfg"
HI3516_FIRMWARE_CFG="/etc/config/firmware.info"
FIRMWARE_PATH="/mnt/mmc1/"

#################################################
#
#  hi3516 config update check daemon script
#  2017/3/21
#  Author: wenkun(wenkun@etonetech.com)
#  2017/06/23
#  fix: LMin(luomin@etonetech.com)
#
#################################################
update_hi3516_config(){
	local cur_md5
	local new_md5

	if [ -f $HI3516_LINPHONE_CFG ]; then
		echo "linphone md5 check..."
		cur_md5=$(md5sum $HI3516_LINPHONE_CFG | awk '{print $1}')
	fi

    if [ -f $TMPFILE_LINPHONE_CFG ]; then
		## if tmpfile is empty, then return 1
		[ "`cat $TMPFILE_LINPHONE_CFG`" = "" ] && return 1
		new_md5=$(md5sum $TMPFILE_LINPHONE_CFG | awk '{print $1}')
		echo  "New:[$new_md5], Old:[$cur_md5]."
		if [ "$cur_md5" = "$new_md5" ]; then
			echo "Config is unchange, do nothing!"
		else
			echo "Config is change, update config file!"
			cp "$TMPFILE_LINPHONE_CFG" "$HI3516_LINPHONE_CFG"
			cp "$HI3516_LINPHONE_CFG" "$HI3516_LINPHONE_RUN_CFG"
            cp "$TMPFILE_IPINTERCOM_CFG" "$HI3516_IPINTERCOM_CFG"
            cp "$TMPFILE_RTSPSERVER_CFG" "$HI3516_RTSPSERVER_CFG"
			return 0
		fi
	fi

    if [ -f $HI3516_IPINTERCOM_CFG ]; then
    		echo "ipintercom md5 check"	
		cur_md5=$(md5sum $HI3516_IPINTERCOM_CFG | awk '{print $1}')
	fi

    if [ -f $TMPFILE_IPINTERCOM_CFG ]; then
		## if tmpfile is empty, then return 1
		[ "`cat $TMPFILE_IPINTERCOM_CFG`" = "" ] && return 1
		new_md5=$(md5sum $TMPFILE_IPINTERCOM_CFG | awk '{print $1}')
		echo  "New:[$new_md5], Old:[$cur_md5]."
		if [ "$cur_md5" = "$new_md5" ]; then
			echo "Config is unchange, do nothing!"
		else
			echo "Config is change, update config file!"
            cp "$TMPFILE_LINPHONE_CFG" "$HI3516_LINPHONE_CFG"
            cp "$HI3516_LINPHONE_CFG" "$HI3516_LINPHONE_RUN_CFG"
            cp "$TMPFILE_IPINTERCOM_CFG" "$HI3516_IPINTERCOM_CFG"
            cp "$TMPFILE_RTSPSERVER_CFG" "$HI3516_RTSPSERVER_CFG"
			return 0
		fi
    fi

    if [ -f $HI3516_RTSPSERVER_CFG ]; then
    		echo "rtsp server md5 check"
		cur_md5=$(md5sum $HI3516_RTSPSERVER_CFG | awk '{print $1}')
	fi

	if [ -f $TMPFILE_RTSPSERVER_CFG ]; then
		## if tmpfile is empty, then return 1
		[ "`cat $TMPFILE_RTSPSERVER_CFG`" = "" ] && return 1
		new_md5=$(md5sum $TMPFILE_RTSPSERVER_CFG | awk '{print $1}')
		echo  "New:[$new_md5], Old:[$cur_md5]."
		if [ "$cur_md5" = "$new_md5" ]; then
			echo "Config is unchange, do nothing!"
		else
			echo "Config is change, update config file!"
            cp "$TMPFILE_LINPHONE_CFG" "$HI3516_LINPHONE_CFG"
            cp "$HI3516_LINPHONE_CFG" "$HI3516_LINPHONE_RUN_CFG"
            cp "$TMPFILE_IPINTERCOM_CFG" "$HI3516_IPINTERCOM_CFG"
            cp "$TMPFILE_RTSPSERVER_CFG" "$HI3516_RTSPSERVER_CFG"
			return 0
		fi
	fi
	return 1
}

check_firmware_version(){
    local cur_firmware_md5
	local new_firmware_md5

    if [ -f $HI3516_FIRMWARE_CFG ]; then
    	echo "rtsp server md5 check"
		cur_firmware_md5=$(md5sum $HI3516_FIRMWARE_CFG | awk '{print $1}')
	fi

	if [ -f $TMPFILE_FIRMWARE_CFG ]; then
		## if tmpfile is empty, then return 1
		[ "`cat $TMPFILE_FIRMWARE_CFG`" = "" ] && return 1
		new_firmware_md5=$(md5sum $TMPFILE_FIRMWARE_CFG | awk '{print $1}')
		echo  "New:[$new_firmware_md5], Old:[$cur_firmware_md5]."
		if [ "$cur_firmware_md5" = "$new_firmware_md5" ]; then
			echo "Config is unchange, do nothing!"
		else
			echo "Config is change, update config file!"
            cp "$TMPFILE_FIRMWARE_CFG" "$HI3516_FIRMWARE_CFG"
			return 0
		fi
	fi
	return 1
}

update_firmware(){
    local FIRMWARE_NAME
    local config
    local Firmware

    FIRMWARE_NAME=$(cat $TMPFILE_FIRMWARE_CFG)
    echo "firmware update:$FIRMWARE_NAME"

    Firmware=${FIRMWARE_PATH}${FIRMWARE_NAME}
    Config=${FIRMWARE_PATH}/target/etc/config

    echo "The update Firmware:$Firmware"

    wget -O ${Firmware} "http://${server}:82/$FIRMWARE_NAME" 2>&1 2> /dev/null

    if [ $? = 0 ]; then
        echo "Get the new firemware success."

        if [ -f $Firmware ]; then
            global_kill
            tar -zvxf $Firmware -C $FIRMWARE_PATH
            cp -rf $Config /etc/
            global_cfg

            return 0
	    fi
    else
        return 1
    fi
}

if [ -f $TMPFILE_IPINTERCOM_CFG ]; then
	rm "$TMPFILE_IPINTERCOM_CFG"
fi

if [ -f $TMPFILE_LINPHONE_CFG ]; then
	rm "$TMPFILE_LINPHONE_CFG"
fi

if [ -f $TMPFILE_RTSPSERVER_CFG ]; then
	rm "$TMPFILE_RTSPSERVER_CFG"
fi



while true
do
    server=192.168.8.10
    echo $server

    wget -O $TMPFILE_FIRMWARE_CFG "http://${server}:82/h3516_firmware.info" 2>&1 2> /dev/null
    if [ $? = 0 ]; then
        check_firmware_version
        if [ $? = 0 ]; then
            update_firmware
            if [ $? = 0 ]; then
                echo "Firmware update success!"
            else
                echo "Firmware update failed!"
            fi
        fi
    fi

	if [ "$server" ]; then
		wget -O $TMPFILE_IPINTERCOM_CFG "http://${server}:82/h3516/IPinterCom.cfg" 2>&1 2> /dev/null
        wget -O $TMPFILE_LINPHONE_CFG   "http://${server}:82/h3516/linphonec.conf" 2>&1 2> /dev/null
        wget -O $TMPFILE_RTSPSERVER_CFG "http://${server}:82/h3516/rtspServer.cfg" 2>&1 2> /dev/null
		ret=$?

		if [ "$ret" = 0 ]; then
			echo "Download config file success."

			#echo $FIRST_FLAG
			if [ "$FIRST_FLAG" = "yes" ]; then

				echo "first run..."
				FIRST_FLAG="no"

				ntpd -p 192.168.8.1

				cp "$TMPFILE_LINPHONE_CFG" "$HI3516_LINPHONE_CFG"
				cp "$HI3516_LINPHONE_CFG" "$HI3516_LINPHONE_RUN_CFG"
				cp "$TMPFILE_IPINTERCOM_CFG" "$HI3516_IPINTERCOM_CFG"
				cp "$TMPFILE_RTSPSERVER_CFG" "$HI3516_RTSPSERVER_CFG"
				global_cfg

                #set timezone
                export TZ=CST-8
			fi

			echo "Download config file success, Ready to check md5."

			update_hi3516_config
			if [ $? = 0 ]; then
				global_cfg
			fi

			echo ""
		fi
	fi

	sleep 2;
done

