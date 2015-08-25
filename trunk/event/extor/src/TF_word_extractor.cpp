#include "TF_word_extractor.h"

TF_word_extractor::TF_word_extractor()
    :base_word_extractor()
{

}

TF_word_extractor::~TF_word_extractor()
{

}

//FUNC  初始化
//IN    _cfg_path: 配置文件路径
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 TF_word_extractor::init(
    var_1* _cfg_path,
    var_vd* _share_pointer /*= NULL*/)
{
    m_status = 0;    

    if (!_cfg_path || !_share_pointer)
    {
        LOG_INVALID_PARAMETER("_cfg_path", "NULL");
        LOG_INVALID_PARAMETER("_share_pointer", "NULL");
        return RET_ERROR_INVALID_PARAM;
    }

    m_share_container = (base_share_container*)_share_pointer;

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
    
    m_features_size = 0;
    m_features_capacity = MAX_NEWS_FEATURE_NUM;
    m_features_pointer = new term_info_st[m_features_capacity];
    if (!m_features_pointer)
    {
        LOG_FAILE_NEW("m_terms_pointer");
        return RET_NO_ENOUGH_MEMORY;
    }

    m_sel_cnt = 0;
    m_sel_features = new term_info_st*[m_features_capacity];
    if (!m_sel_features)
    {
        LOG_FAILE_NEW("m_sel_features");
        return RET_NO_ENOUGH_MEMORY;
    }

    ret = m_feature_map.InitHashSearch(MAX_NEWS_FEATURE_NUM * 100, sizeof(term_info_st*));
    if (ret)
    {
        LOG_FAILE_CALL_RET("TF_word_extractor::init", "m_feature_map.InitHashSearch", ret);
        return RET_NO_ENOUGH_MEMORY;
    }

    SET_FLAG(m_status, WE_init);

    return 0;
}

//FUNC  执行抽取
//IN    _doc_pointer: 待抽取的文本指针
//IN    _doc_len: 待抽取的文本长度
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 TF_word_extractor::extract(
    var_u4 _doc_len, 
    var_1* _doc_pointer)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "TF_word_extractor::init");
        return RET_ERROR_INVALID_PARAM;    
    }
    RESET_FLAG(m_status, WE_EXTR);
    m_features_size = 0;
    m_feature_map.ClearHashSearch();

    var_4 ret = base_word_extractor::parse_news_buf(_doc_pointer, _doc_len);
    if (ret)
    {
        LOG_FAILE_CALL_RET("TF_word_extractor::extract", "base_word_extractor::parse_news_buf", ret);
        return RET_ERROR_INVALID_PARAM;            
    }

    ret = calc_weight(m_tit_inf.left, m_tit_inf.right, m_base_conf._tit_weight);
    if (ret)
    {
        LOG_FAILE_CALL_RET("TF_word_extractor::extract", "TF_word_extractor::calc_weight", ret);
        return RET_ERROR_INVALID_PARAM;                    
    }

    ret = calc_weight(m_text_inf.left, m_text_inf.right, 1);
    if (ret)
    {
        LOG_FAILE_CALL_RET("TF_word_extractor::extract", "TF_word_extractor::calc_weight", ret);
        return RET_ERROR_INVALID_PARAM;                    
    }

//     if (m_features_size < m_base_conf._feature_min_num)
//     {//! 关键词个数太少，抛弃掉（常常是英文新闻、乱码新闻、无正文、短新闻）
//         return RET_FALSE;
//     }

    ret = select_feature();
    if (ret)
    {// 计算权重，选特征词
        LOG_FAILE_CALL_RET("TF_word_extractor::extract", "TF_word_extractor::select_feature", ret);
        return RET_ERROR_INVALID_PARAM;  
    }

    ret = m_share_container->exist_nid(m_news_ID);
    if (!ret)
    {// 不存在， 更新DF
        ret = m_share_container->update_DF(m_sel_features, m_sel_cnt);
        if (ret)
        {
            LOG_FAILE_CALL_RET("TF_word_extractor::extract", "m_share_container->update_DF", ret);
            return RET_ERROR_INVALID_PARAM;  
        }
        ret = m_share_container->add_nid(m_news_ID);
        if (ret)
        {
            LOG_FAILE_CALL_RET("TF_word_extractor::extract", "m_share_container->add_nid", ret);
            return RET_ERROR_INVALID_PARAM;  
        }
    }

    SET_FLAG(m_status, WE_EXTR);

    return RET_SECCEED;
}

//FUNC  获取词语信息
//IN    _calc_type: 待获取的词语类型
//OUT   _term_pointer: 待写入词语信息缓冲区
//IN    _term_cnt: 待写入的最大词语信息个数
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 TF_word_extractor::get_term(
    word_type_e _type, 
    term_info_st* _term_pointer, 
    var_u4 _term_cnt)
{
    term_info_st* pos_wt = _term_pointer;
    term_info_st* end_wt = _term_pointer + _term_cnt;

    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;

    for (; end > pos && end_wt > pos_wt; ++pos)
    {
        if ((_type & pos->type) || (_type == WT_ALL))
        {
            *pos_wt++ = *pos;
        }  
    }

    return pos_wt - _term_pointer;
}

//FUNC  获取词语个数
//IN    _calc_type: 待获取的词语类型
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 TF_word_extractor::get_count(
    word_type_e _type)
{
    if (TEST_FLAG(_type, WT_ALL))
    {
        return m_features_size;    
    }

    var_4 ret = 0;
    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;
    for (; end > pos; ++pos)
    {
        if ((_type & pos->type) || (_type == WT_ALL))
        {
            ++ret;
        }  
    }

    return ret;
}

//FUNC  获取错误信息
//IN    _err_code: 错误码
//RET   0：正常 其他：获取到的错误信息
var_1* TF_word_extractor::get_error_description(
    var_4 _err_code)
{
    return 0;
}

var_vd TF_word_extractor::uninit()
{
    base_word_extractor::_uninit();
}

var_4 TF_word_extractor::need_consider(var_1* _word)
{
    var_u4 len = 0u;
    var_u4 cnt = 0u;
    var_1* pos = _word;
    for (; *pos;)
    {
        if (0 > *pos)
        {
            ++pos;
            if (!*pos)
            {
                return 0;   
            }
            ++len;
        }

        ++pos;
        ++len;
        ++cnt;
    }

    if (3 > len || 2 > cnt)
    {
        return 0;
    }

    if (badcode(_word))
    {
        return 0;
    }

    if (m_share_container->stop_word(_word))
    {
        return 0;
    }

    return 1;    
}

var_4 TF_word_extractor::calc_weight(var_1* _doc_beg, var_1* _doc_end, var_u2 _factor)
{
    assert(_doc_beg);
    assert(_doc_end);
    if (_doc_beg == _doc_end)
    {
        return RET_SECCEED;    
    }

    var_1* text = _doc_beg;
    if (m_base_conf._self_cut)
    {
        *_doc_end = '\0';
        text = (var_1*)NLPIR_ParagraphProcess(_doc_beg);
        assert(text);
        
        for (_doc_end = text; *_doc_end; ++_doc_end);
    }

    var_u8 word_id = 0u;
    var_u8 tag_id = 0u;
    var_1* word_beg = NULL;
    var_1* word_end = NULL;
    var_1* tag_beg = NULL;
    var_1* tag_end = NULL;

    var_1* pos = text;
    var_u4 space_len = 0u;

    var_1 word[MAX_KEYWORD_LEN + 1] = {""};
    var_4 word_len = 0;
    var_f4 weight = 0.f;

    var_4 ret = 0;
    tagweight* tmp_TW = NULL;
    term_info_st* cur_feature = NULL;
    term_info_st** pre_feature = NULL;

    var_1 tag[100];
    while (_doc_end > pos)
    {
        //过滤前导空白
        while (_doc_end > pos)
        {
            space_len = len_start_space(pos);
            if (0 < space_len)
            {
                pos += space_len;
            }
            else
            {
                break;
            }
        }
        word_beg = pos;

        //识别词语
        space_len = 0;
        while (_doc_end > pos)
        {
            if (0 > *pos)
            {
                if (' ' == *pos) // 此处不能是中文全角空格，中科院分词决定
                {
                    space_len = 1;
                    break;
                }
                ++pos;
                if (_doc_end == pos)
                {
                    break;
                }
            }
            else if (' ' == *pos)
            {
                break;
            }
            
            ++pos;
        }
        tag_end = pos;
           
        //识别词性
        var_1* p = pos;
        for (; p != word_beg && '/' != *p; --p);

        if (p == word_beg)
        {
            continue;
        }
        word_end = p;
        tag_beg = p + 1;
        
        if (word_beg < word_end && tag_beg < tag_end)
        {
            word_len = tag_end - tag_beg;
            if (100 < word_len)
            {
                continue;
            }
            memcpy(tag, tag_beg, word_len);
            tag[word_len] = '\0';

            tag_id = m_md5er.MD5Bits64((var_u1*)tag, word_len);

            word_len = word_end - word_beg;
            if (MAX_KEYWORD_LEN < word_len)
            {
                continue;
            }
            memcpy(word, word_beg, word_len);
            word[word_len] = '\0';

            word_id = m_md5er.MD5Bits64((var_u1*)word, word_len);

            if (need_consider(word))
            {// 处理该词语（也保留以英文字符开头的词语）

                //根据词性计算权重
                var_4 tmp_TW = m_share_container->get_weight(tag_id);
                if (0 < tmp_TW)
                {
                    weight = tmp_TW;
                    weight *= _factor;

                    if (m_features_size == m_features_capacity)
                    {
                        LOG_ERROR("文本过长", "特征词缓冲区不够用！！！");
                        break;
                    }
                    cur_feature = m_features_pointer + m_features_size;

                    cur_feature->weight = weight;
                    cur_feature->word_id = word_id;
                    memcpy(cur_feature->word_str, word, word_len + 1);
                    cur_feature->type = 0;
                    ret = assign_word_type(cur_feature->type, tag);
                    if (ret)
                    {
                        LOG_FAILE_CALL_RET("TF_word_extractor::calc_weight", "assign_word_type", ret);
                        return RET_ERROR_INVALID_PARAM;
                    }
                    
                    ret = m_feature_map.AddKey_FL(word_id, &cur_feature, (var_vd**)&pre_feature);
                    if (0 > ret)
                    {
                        LOG_FAILE_CALL_RET("TF_word_extractor::calc_weight", "m_feature_map->AddKey_FL", ret);
                        return RET_ERROR_INVALID_PARAM;
                    }
                    else if (1 == ret)
                    {// 已存在 
                        assert(*pre_feature);
                        assert(0.f < (*pre_feature)->weight);
                        assert((*pre_feature)->word_str[0]);
                        (*pre_feature)->weight += weight;
                        (*pre_feature)->type |= cur_feature->type;
                        ++(*pre_feature)->tf;
                    }
                    else
                    {
                        cur_feature->tf = 1;
                        ++m_features_size;
                    }

                }
            }
        }
        else
        {
            assert(!"警告：无词语或无词性");
        }

        pos += space_len; // 跳过当前空白字符
    }
    
    return RET_SECCEED;
}

// IDF排降序，相同IDF排字典序
static bool feature_cmp(const term_info_st& lhs, const term_info_st& rhs)
{
    if (rhs.weight < lhs.weight)
    {
        return false;
    }
    else if (rhs.weight == lhs.weight)
    {
        if (0 >= strcmp(rhs.word_str, lhs.word_str))
        {
            return false;
        }
    }
    else //(rhs.weight > lhs.weight)
    {
        ;
    }

    return true;
}

var_4 TF_word_extractor::select_feature()
{
    var_u4 idx = 0u;
    // 求最大weight值
    var_f4 max_WI = 0.f;
    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;
    for (; end > pos; ++pos)
    {
        if (pos->weight > max_WI)
        {
            max_WI = pos->weight;
        }  
    }

    // 计算权重
    pos = m_features_pointer;
    for (; end > pos; ++pos)
    {
        pos->idf = m_share_container->get_IDF(pos->word_id);
        // 公式2.4 《Modern Information Retrieval》P30.
        //! 其中idf采用lucene公式计算
        pos->weight = (0.5f + 0.5f * (pos->weight / max_WI)) * pos->idf;
    }
#define PRINTF
#ifdef PRINTF
{
    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;
    var_4 idx = 1;
    printf("-----------------------------排序前---------------------\n");
    for (; end > pos; ++pos)
    {
        printf("%d:\t词语[%s]\t权重[%.2f]\t词性[%d]\tTF[%d]\tIDF[%.2f]\n", idx++, pos->word_str, 
            pos->weight, pos->type, pos->tf, pos->idf);
    }
}
#endif
    // 按权值排降序
    _quick_sort<term_info_st>(m_features_pointer, 0, m_features_size - 1, feature_cmp);
#ifdef PRINTF
{
    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;
    var_4 idx = 1;
    printf("-----------------------------排序后---------------------\n");
    for (; end > pos; ++pos)
    {
        printf("%d:\t词语[%s]\t权重[%.2f]\t词性[%d]\tTF[%d]\tIDF[%.2f]\n", idx++, pos->word_str, 
            pos->weight, pos->type, pos->tf, pos->idf);
    }
}
#endif

    if (m_base_conf._feature_max_num < m_features_size)
    {
        m_features_size = m_base_conf._feature_max_num;
    }

    // 向量归一化
    normalize_vector(m_features_pointer, m_features_size);

    return RET_SECCEED;
}
