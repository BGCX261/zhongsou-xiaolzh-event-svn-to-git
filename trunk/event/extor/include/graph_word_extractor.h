#ifndef __GRAPH_WORD_EXTRACTOR_H_
#define __GRAPH_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "UC_ReadConfigFile.h"
#include "base_word_extractor.h"

class graph_word_extractor : public base_word_extractor
{
public:
    graph_word_extractor();

	virtual ~graph_word_extractor();

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

};

#endif

