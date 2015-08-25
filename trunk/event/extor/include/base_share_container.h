#ifndef __BASE_SHARE_CONTAINER_H_
#define __BASE_SHARE_CONTAINER_H_
#include "ext_common_def.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "UC_ReadConfigFile.h"
#include "word_IDF_calcor.h"
#include "UC_MD5.h"

class word_IDF_calcor;
class UC_ReadConfigFile;
class base_share_container
{
public:
    base_share_container();

	~base_share_container();

    //FUNC  初始化
    //IN    _cfg_path: 配置文件路径
    //RET   0：正常 其他：错误码
	var_4 init(
        var_1* _cfg_path
        );

    //FUNC  反初始化
    //RET   空
    var_vd uninit();

    //FUNC  查看是否存在该newsID
    //IN    _NID:newsID
    //RET   1：存在 0：不存在
    var_4 exist_nid(
        const var_u8 _NID
        );

    //FUNC  添加该newsID
    //IN    _NID:newsID
    //RET   0：添加成功 其他：不成功
    var_4 add_nid(
        const var_u8 _NID
        );

    var_4 inc_totalDF();

    var_4 update_DF(
        term_info_st** _words, 
        var_u4 _word_cnt
        );

    var_f4 get_IDF(
        var_u8 _word_id
        );

    var_4 get_weight(
        var_u8 tag_id
        );

    var_4 stop_word(
        var_1* _word
        );

private:
    var_4 load_key_weight(
        UC_ReadConfigFile* _reader
        );

    var_4 load_stop_word(
        UC_ReadConfigFile* _reader
        );

private:
    var_4 m_status;
    //CP_MutexLock_RW             m_lck;
    word_IDF_calcor             m_idf_calcor;
    UT_HashTable_Pro<var_u8>    m_NID_hash;

    
    //存储 词性权重映射关系<var_u8, tagweight*>
    UT_HashSearch<var_u8>       m_tagweight_map;

    UT_HashSearch<var_u8>       m_stop_hash;

    var_1                       m_TF_data[256];

    UC_MD5                      m_md5er;
    //var_4   m_ref;
};

#endif
