#ifndef __BASE_WORD_EXTRACTOR_H_
#define __BASE_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "UC_ReadConfigFile.h"
#include "ext_common_def.h"
#include "UC_MD5.h"
#include "UT_HashSearch.h"

typedef struct tag_base_conf
{
    var_1 _self_cut : 1;        //0-> �ı�δ�дʣ���ϵͳ�Լ��д�
    
    var_1 _trunc_tit : 1;       //1-> �������ȡ
    var_1 _tit_max_len : 6;     // �����ȡ��󳤶�
    var_u1 _tit_weight;         // // ������������г���ʱ�����ӵ�Ȩ��[0~255]��5�����൱���������г���5�Σ�

    var_4 _trunc_text : 1;      //1-> �������ȡ
    var_4 _text_max_len : 31;    // ���Ľ�ȡ��󳤶�

    //var_u2 _feature_min_num;    //���������ʸ��������ı���������ȡʧ�� ��������Ӣ�����š��������š������ġ������ţ�
    var_u2 _feature_max_num;
    
    var_1 _zky_data[256];
}base_conf, *pbase_conf;

class UC_ReadConfigFile;

class base_word_extractor
{
public:
    base_word_extractor();

    virtual ~base_word_extractor();

    //FUNC  ��ʼ��
    //IN    _cfg_path: �����ļ�·��
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
	virtual var_4 init(var_1* _cfg_path, var_vd* _share_pointer = NULL) = 0;

    //FUNC  ����ʼ��
    //RET   ��
    virtual var_vd uninit() = 0;

    //FUNC  ִ�г�ȡ
    //IN    _doc_pointer: ����ȡ���ı�ָ�� ��Ѷ��ʽ��docid(8)+titlen(4)+title(titlen)+doclen(4)+doc(doclen)
    //IN    _doc_len: ����ȡ���ı�����       
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    virtual var_4 extract(var_u4 _doc_len, var_1* _doc_pointer) = 0;
	
    //FUNC  ��ȡ������Ϣ
    //IN    _calc_type: ����ȡ�Ĵ�������
    //OUT   _term_pointer: ��д�������Ϣ������
    //IN    _term_cnt: ��д�����������Ϣ����
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    virtual var_4 get_term(word_type_e _type, term_info_st* _term_pointer, var_u4 _term_cnt) = 0;
	
    //FUNC  ��ȡ�������
    //IN    _calc_type: ����ȡ�Ĵ�������
    //RET   0������ ������������ ����get_error_description��ȡ������Ϣ
    virtual var_4 get_count(word_type_e _type) = 0;

    //FUNC  ��ȡ������Ϣ
    //IN    _err_code: ������
    //RET   0������ ��������ȡ���Ĵ�����Ϣ
    virtual var_1* get_error_description(var_4 _err_code) = 0;

protected:
    var_4 parse_news_buf(
        var_1* _news_buf, 
        var_u4 _news_len
        );

    var_4 _init(
        UC_ReadConfigFile* _reader
        );

    var_vd _uninit();

    var_vd normalize_vector(
        term_info_st* _features, 
        var_u4 _cnt
        );

private:
    var_4 read_conf(
        UC_ReadConfigFile* _reader
        );

protected:
    var_u4          m_status;
    base_conf       m_base_conf;
    simple_pair<var_1*, var_1*> m_tit_inf;
    simple_pair<var_1*, var_1*> m_text_inf;
    var_u8          m_news_ID;

    UC_MD5 m_md5er;

    //�洢��ȡ������������Ϣ<var_u8, feature*>
    UT_HashSearch<var_u8> m_feature_map;
};

#endif

