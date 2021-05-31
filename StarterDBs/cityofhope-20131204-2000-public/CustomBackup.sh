#!/bin/sh
#
# Standard MUSH backup script modified by bem@erisian.net to work with MUX
#
PATH=/bin:/usr/bin:/usr/local/bin:.; export PATH
BASEDIR=/opt/tinymux/coh
BACKUPDIR=${BASEDIR}/game/backups
OUTPUTDIR=${BACKUPDIR}/frequent

cd ${BASEDIR}/game
#
. mux.config
#
# You'll want to use gzip if you have it. If you want really good
# compression, try 'gzip --best'. If you don't have gzip, use 'compress'.
ZIP=bzip2
ENCRYPT="gpg -e -r sponge@cityofhopemush.com"
#
DBDATE=`date +%Y%m%d-%H%M`
#
# files to tar up
#TEXT_FILES="$TEXT/badsite.txt $TEXT/connect.txt $TEXT/create_reg.txt $TEXT/down.txt $TEXT/full.txt $TEXT/guest.txt $TEXT/motd.txt $TEXT/news.txt $TEXT/newuser.txt $TEXT/plushelp.txt $TEXT/quit.txt $TEXT/register.txt $TEXT/wizmotd.txt $TEXT/wiznews.txt $TEXT/staffhelp.txt"
TEXT_FILES=$TEXT/*

CONFIG_FILES="mux.config $GAMENAME.conf backup_rotation.conf"

ADMIN_FILES="CustomBackup.sh /usr/local/bin/rotate_archives.pl /etc/cron.d/tinymux ${BASEDIR}/../.ssh/"

GAME_DB=$DATA/$GAMENAME.FLAT

if [ -r $DATA/comsys.db ]; then
    COMM_DB=$DATA/comsys.db
fi

if [ -r $DATA/mail.db ]; then
    MAIL_DB=$DATA/mail.db
fi

if [ -r $DATA/$NEW_DB ]; then
    $BIN/dbconvert -d$DATA/$GDBM_DB -u -i$DATA/$NEW_DB -o$DATA/$GAMENAME.FLAT
else
    if [ -r $DATA/$INPUT_DB ]; then
        echo "No recent checkpoint db. Using older db."
        $BIN/dbconvert -d$DATA/$GDBM_DB -u -i$DATA/$INPUT_DB -o$DATA/$GAMENAME.FLAT
    else
        if [ -r $DATA/$SAVE_DB ]; then
            echo "No input db. Using backup db."
            $BIN/dbconvert -d$DATA/$GDBM_DB -u -i$DATA/$SAVE_DB -o$DATA/$GAMENAME.FLAT
        else
            echo "No dbs. Backup attempt failed."
	    exit
        fi
    fi
fi

tar cfp - $GAME_DB $COMM_DB $MAIL_DB $CONFIG_FILES $TEXT_FILES $ADMIN_FILES | $ZIP -c | ${ENCRYPT} > $OUTPUTDIR/$GAMENAME.$DBDATE.tar.bz2.gpg
rm $DATA/$GAMENAME.FLAT
/usr/local/bin/rotate_archives.pl ${BASEDIR}/game/backup_rotation.conf
rsync -auH --delete ${BACKUPDIR}/ muxbackups@renfield.lub-dub.org:cityofhope/
