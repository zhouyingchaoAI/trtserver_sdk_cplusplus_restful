#ifndef __UTDNN_H__
#define __UTDNN_H__

#ifdef _WIN32

#ifdef UTDNN_EXPORTS
#define UTDNN_API __declspec(dllexport)
#else
#define UTDNN_API __declspec(dllimport)
#endif

#else
#define UTDNN_API
#endif

#define DNN_SERVER_TRT (0)
#define DNN_SERVER_TFS (1)

typedef struct
{
    int  version;   //模型版本号
    int  type;
    int  state;     //此版本是否在线可用, 0 = 不可用， 1 = 可用
}MODELINFO;

typedef struct
{
    int   cid;        //目标类别id
    float score;      //目标相似度
    float tlx;        //左上角x坐标(top-left)
    float tly;        //左上角y坐标(top-left)
    float brx;        //右下角x坐标(bottom-right)
    float bry;        //右下角y坐标(bottom-right)
}DNNTARGET;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 设置或修改深度学习服务器地址
 * @param [in] pdnn    dnn对象，若不为空则为修改深度学习服务器地址
 * @param [in] srvip   深度学习服务器IP
 * @param [in] srvport 深度学习服务器端口
 * @return void
 */
UTDNN_API void dnn_init(void * pdnn, const char * srvip, unsigned int srvport);

/**
 * @brief 获取模型版本信息、测试深度学习服务器连接状态
 * @param [in, out] versions  各版本状态信息，空间由调用者申请
 * @param [in, out] ncnt      [in]调用者申请的MODELINFO个数；[out]查询到的MODELINFO个数，IP/端口错误、模型或指定版本号不存在时返回0
 * @param [in]      modelname 模型名字
 * @param [in]      version   需查询的模型版本，-1 = 查询该模型所有版本状态
 * @return void
 */
UTDNN_API void  dnn_getstate(MODELINFO * versions, int * ncnt, const char * modelname, int version = -1);

/**
 * @brief 添加深度学习服务请求对象
 * @param [in] modelname 模型名(使用该模型的最新版本号)
 * @return void
 */
UTDNN_API void * dnn_addconn(const char * modelname);

/**
 * @brief 单帧数据请求
 * @param [in] devip        NVR IP
 * @param [in] chn          通道号
 * @param [in] modelname    模型名
 * @param [in] data         图片数据
 * @param [in， out] objs   请求结果
 * @param [in， out] size   结果数量
 */

UTDNN_API void  dnn_predict(void * pdnn, unsigned char * data, int w, int h, DNNTARGET * objs, int *size, int jpgsize=0);

/**
 * @brief 删除深度学习服务请求对象
 * @param [in] devip     NVR IP
 * @param [in] chn       通道号
 * @param [in] modelname 模型名
 * @return void
 */
UTDNN_API void  dnn_rmconn(void * pdnn);

#ifdef __cplusplus
}
#endif

#endif //__UTDNN_H__
