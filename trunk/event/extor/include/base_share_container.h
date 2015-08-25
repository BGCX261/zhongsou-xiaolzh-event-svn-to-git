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

    //FUNC  ��ʼ��
    //IN    _cfg_path: �����ļ�·��
    //RET   0������ ������������
	var_4 init(
        var_1* _cfg_path
        );

    //FUNC  ����ʼ��
    //RET   ��
    var_vd uninit();

    //FUNC  �鿴�Ƿ���ڸ�newsID
    //IN    _NID:newsID
    //RET   1������ 0��������
    var_4 exist_nid(
        const var_u8 _NID
        );

    //FUNC  ��Ӹ�newsID
    //IN    _NID:newsID
    //RET   0����ӳɹ� ���������ɹ�
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

    
    //�洢 ����Ȩ��ӳ���ϵ<var_u8, tagweight*>
    UT_HashSearch<var_u8>       m_tagweight_map;

    UT_HashSearch<var_u8>       m_stop_hash;

    var_1                       m_TF_data[256];

    UC_MD5                      m_md5er;
    //var_4   m_ref;
};

#endif
