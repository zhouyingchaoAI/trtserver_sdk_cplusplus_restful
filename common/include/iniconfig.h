#ifndef __INI_CONFIG_H__
#define __INI_CONFIG_H__

/** < version.ini */
#define VER_CONFIG         "./version.ini"
#define SEC_VERSION        "ServerVersion"

#define ID_VERSION_KEY     "version"
#define ID_VERSION_VAL     "1.0"

#define ID_SVN_KEY         "svn"
#define ID_SVN_VAL         "1500"

/** < config.ini */
#define INI_CONFIG         "./config.ini"
    /** < Global */
#define SEC_GLOBAL         "Global"

#define ID_ONTURN_KEY      "IsTurnRun"
#define ID_ONTURN_VAL      "0"

#define ID_TCPPORT_KEY     "tcpport"
#define ID_TCPPORT_VAL     "5000"

#define ID_HTTPPORT_KEY    "httpport"
#define ID_HTTPPORT_VAL    "7600"

#define ID_CONSOLE_KEY     "IsShowConsole"
#define ID_CONSOLE_VAL     "1"

#define ID_SHOWIMG_KEY     "IsShowImage"
#define ID_SHOWIMG_VAL     "1"

#define ID_STREAM_KEY      "IsSubStream"
#define ID_STREAM_VAL      "1"

#define ID_SAVEIMG_KEY     "IsSaveImage"
#define ID_SAVEIMG_VAL     "1"

#define ID_DELNUM_IMG_KEY  "ImageDelNum"
#define ID_DELNUM_IMG_VAL  "5000"

#define ID_DELAYSEC_KEY    "PresetDelaySec"
#define ID_DELAYSEC_VAL    "0"

#define ID_TURN_RUN_KEY    "IsTurnRun"
#define ID_TURN_RUN_VAL    "0"

#define ID_TFSERVIP_KEY    "TFServerIP"
#define ID_TFSERVIP_VAL    "127.0.0.1"

#define ID_CONTAINER_KEY   "N_TFS_Container"
#define ID_CONTAINER_VAL   "1"

#define ID_ALARMIM_DIR_KEY "AlarmImDir"
#define ID_ALARMIM_DIR_VAL "./AlarmImages/"

#define ID_FACEIM_DIR_KEY  "FaceImgDir"
#define ID_FACEIM_DIR_VAL  "./face/images/"

    /** < SAFTYRULE */
#define SEC_SAFTY          "SUPER_WORKING"
#define ID_HELMETC_KEY      "Helmet_color"
#define ID_HELMETC_VAL      "蓝_红_黄_白"

#define ID_CLOTHC_KEY      "Cloth_color"
#define ID_CLOTHC_VAL      "蓝"
 
#define ID_CLOTHT_KEY      "Cloth_thre"
#define ID_CLOTHT_VAL      "0.5"

#define ID_BELTC_KEY       "Belt_Color"
#define ID_BELTC_VAL       "红"

#define ID_BELTT_KEY       "Belt_thre"
#define ID_BELTT_VAL       "0.18"

#endif
