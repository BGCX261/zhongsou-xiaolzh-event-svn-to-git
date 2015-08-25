#ifndef __TF_WORD_EXTRACTOR_H_
#define __TF_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "ext_common_def.h"
#include "UT_Allocator.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "word_IDF_calcor.h"
#include "base_share_container.h"
#include "UC_ReadConfigFile.h"
#include "base_word_extractor.h"

class base_share_container;

class TF_word_extractor : public base_word_extractor
{
public:
    TF_word_extractor();

	virtual ~TF_word_extractor();

    //FUNC  ��ʼ��
    //IN    _cfg_path: �����ļ�·��
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
	var_4 init(
        var_1* _cfg_path,
        var_vd* _share_pointer = NULL
        );

    //FUNC  ����ʼ��
    //RET   ��
    virtual var_vd uninit();
	
    //FUNC  ִ�г�ȡ
    //IN    _doc_pointer: ����ȡ���ı�ָ��
    //IN    _doc_len: ����ȡ���ı�����
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    var_4 extract(
        var_u4 _doc_len, 
        var_1* _doc_pointer
        );
	
    //FUNC  ��ȡ������Ϣ
    //IN    _calc_type: ����ȡ�Ĵ�������
    //OUT   _term_pointer: ��д�������Ϣ������
    //IN    _term_cnt: ��д�����������Ϣ����
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    var_4 get_term(
        word_type_e _type, 
        term_info_st* _term_pointer, 
        var_u4 _term_cnt
        );
	
    //FUNC  ��ȡ�������
    //IN    _calc_type: ����ȡ�Ĵ�������
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    var_4 get_count(
        word_type_e _type
        );

    //FUNC  ��ȡ������Ϣ
    //IN    _err_code: ������
    //RET   0������ ��������ȡ���Ĵ�����Ϣ
    var_1* get_error_description(
        var_4 _err_code
        );

private:
    var_4 calc_weight(
        var_1* _doc_beg, 
        var_1* _doc_end, 
        var_u2 _factor
        );

    var_4 select_feature();

    var_4 need_consider(
        var_1* _word
        );

private:
    term_info_st*    m_features_pointer;
    var_u4      m_features_capacity;
    var_u4      m_features_size;
    base_share_container* m_share_container;

    term_info_st**   m_sel_features;
    var_u4      m_sel_cnt;
};

#endif

