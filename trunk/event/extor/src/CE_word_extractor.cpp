#include "CE_word_extractor.h"

CE_word_extractor::CE_word_extractor()
{

}

CE_word_extractor::~CE_word_extractor()
{

}

//FUNC  ��ʼ��
//IN    _cfg_path: �����ļ�·��
//RET   0������ ������������ ����get_error_description��ȡ������Ϣ
var_4 CE_word_extractor::init(
    var_1* _cfg_path,
    var_vd* _share_pointer /*= NULL*/)
{
    m_status = 0;    

    UC_ReadConfigFile cur_reader;
    var_4 ret = cur_reader.InitConfigFile(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("TF_word_extractor::init", "cur_reader.InitConfigFile", _cfg_path);
        return RET_ERROR_INVALID_PARAM;
    }
        
    ret = base_word_extractor::_init(&cur_reader);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("TF_word_extractor::init", "base_word_extractor::read_conf", _cfg_path);
        return RET_ERROR_INVALID_PARAM;        
    }

    SET_FLAG(m_status, WE_init);

    return 0;
}

//FUNC  ִ�г�ȡ
//IN    _doc_pointer: ����ȡ���ı�ָ��
//IN    _doc_len: ����ȡ���ı�����
//RET   0������ ������������ ����get_error_description��ȡ������Ϣ
var_4 CE_word_extractor::extract(
    var_u4 _doc_len, 
    var_1* _doc_pointer)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("���ȵ���", "TF_word_extractor::init");
        return RET_ERROR_INVALID_PARAM;    
    }
    RESET_FLAG(m_status, WE_EXTR);

    var_4 ret = base_word_extractor::parse_news_buf(_doc_pointer, _doc_len);
    if (ret)
    {
        LOG_FAILE_CALL_RET("graph_word_extractor::extract", "base_word_extractor::parse_news_buf", ret);
        return RET_ERROR_INVALID_PARAM;            
    }

    {
        ///////////////////////////��ȡ����//////////////////////////////////////
    }

    SET_FLAG(m_status, WE_EXTR);

    return 0;
}

//FUNC  ��ȡ������Ϣ
//IN    _calc_type: ����ȡ�Ĵ�������
//OUT   _term_pointer: ��д�������Ϣ������
//IN    _term_cnt: ��д�����������Ϣ����
//RET   0������ ������������ ����get_error_description��ȡ������Ϣ
var_4 CE_word_extractor::get_term(
    word_type_e _type, 
    term_info_st* _term_pointer, 
    var_u4 _term_cnt)
{
    return 0;
}

//FUNC  ��ȡ�������
//IN    _calc_type: ����ȡ�Ĵ�������
//RET   0������ ������������ ����get_error_description��ȡ������Ϣ
var_4 CE_word_extractor::get_count(
    word_type_e _type)
{
    return 0;
}

//FUNC  ��ȡ������Ϣ
//IN    _err_code: ������
//RET   0������ ��������ȡ���Ĵ�����Ϣ
var_1* CE_word_extractor::get_error_description(
    var_4 _err_code)
{
    return 0;
}

var_vd CE_word_extractor::uninit()
{
    base_word_extractor::_uninit();
}

