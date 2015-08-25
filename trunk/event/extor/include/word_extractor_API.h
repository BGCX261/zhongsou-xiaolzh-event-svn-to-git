#ifndef __WORD_EXTRACTOR_API_H_
#define __WORD_EXTRACTOR_API_H_

#include "TF_word_extractor.h"
#include "graph_word_extractor.h"
#include "CE_word_extractor.h"
#include "zky_word_extractor.h"
#include "ext_common_def.h"
#include "UT_Allocator.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "word_IDF_calcor.h"
#include "base_share_container.h"
#include "base_word_extractor.h"

/*************************************************  get_dict_handle ******************************************************
//����:�õ��ֵ���
//����ֵ���ɹ����ؾ����ʧ�ܷ���NULL
//����˵����
	_cfg_path       ����·��
******************************************************************************************************************/
var_vd* get_dict_handle(var_1* _cfg_path);

#define TF_EXTRACTOR        0
#define GRAPH_EXTRACTOR     1
#define CE_EXTRACTOR        2
#define ZKY_EXTRACTOR       3
/*************************************************  get_extor_handle ******************************************************
//����:�õ���ȡ�����
//����ֵ���ɹ����ؾ����ʧ�ܷ���NULL
//����˵����
    _cfg_path       ����·��
	_WE_type		��ȡ������
    _dict_handle    �ֵ���
******************************************************************************************************************/
base_word_extractor* get_extor_handle(var_1* _cfg_path, var_4 _WE_type, var_vd* _dict_handle = NULL);

#define free_dict_handle(x) free_extor_dict_handle(x, NULL)
#define free_extor_handle(x) free_extor_dict_handle(NULL, x)

/*************************************************  free_extor_dict_handle *****************************************************
//����:�ͷ�ʹ�õľ��
//����ֵ����
//����˵����
	_dict_handle        �ֵ���
	base_word_extractor ��ȡ�����		
******************************************************************************************************************/
var_vd free_extor_dict_handle(var_vd* _dict_handle, base_word_extractor* _extor_handle);


#endif
