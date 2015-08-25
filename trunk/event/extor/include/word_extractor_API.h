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
//功能:得到字典句柄
//返回值：成功返回句柄，失败返回NULL
//参数说明：
	_cfg_path       配置路径
******************************************************************************************************************/
var_vd* get_dict_handle(var_1* _cfg_path);

#define TF_EXTRACTOR        0
#define GRAPH_EXTRACTOR     1
#define CE_EXTRACTOR        2
#define ZKY_EXTRACTOR       3
/*************************************************  get_extor_handle ******************************************************
//功能:得到提取器句柄
//返回值：成功返回句柄，失败返回NULL
//参数说明：
    _cfg_path       配置路径
	_WE_type		提取器类型
    _dict_handle    字典句柄
******************************************************************************************************************/
base_word_extractor* get_extor_handle(var_1* _cfg_path, var_4 _WE_type, var_vd* _dict_handle = NULL);

#define free_dict_handle(x) free_extor_dict_handle(x, NULL)
#define free_extor_handle(x) free_extor_dict_handle(NULL, x)

/*************************************************  free_extor_dict_handle *****************************************************
//功能:释放使用的句柄
//返回值：空
//参数说明：
	_dict_handle        字典句柄
	base_word_extractor 提取器句柄		
******************************************************************************************************************/
var_vd free_extor_dict_handle(var_vd* _dict_handle, base_word_extractor* _extor_handle);


#endif
