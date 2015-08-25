#ifndef __EVENT_CONTAINER_H_
#define __EVENT_CONTAINER_H_
#include <set>
#include "UH_Define.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "UC_ReadConfigFile.h"
#include "UC_Allocator_Recycle.h"
#include "UT_Persistent_KeyValue.h"

#include "whl_list.h"

#include "word_extractor_API.h"

#include "common_def.h"
#include "utility.h"

#include "doc_infer.h"
#include "event_infer.h"


class UC_ReadConfigFile;
class UC_Allocator_Recycle;
class event_infer;
class doc_infer;

#ifdef _SYSTEM_X86_

typedef var_u4 var_p;

#else

typedef var_u8 var_p;

#endif // _SYSTEM_X86_

// calculate the possibility of the doc belong to this event 
struct doc_evt_dst
{
	event_infer* _einfer;	// event
	var_f4 _score;			// the score this doc belong to _einfer
	var_f4 _max_doc_score;	// the max score of [this doc] and [one of docs belong to _einfer]
	var_u1 _doc_cnt;		// the number of docs belong to _einfer
};

class event_container
{
public:
	event_container();
	
    ~event_container();
	
    var_4 add_doc( 
        doc_infer* _dinfer
        );

    var_4 del_doc( 
        const var_u8 _did
        );

    var_4 del_evt( 
        const var_u8 _evt_ID
        );
    
    var_4 init(
        UC_ReadConfigFile* _reader,
        UT_Persistent_KeyValue<var_u8>* _storer,
        const var_4 _start_opt
        );

    var_vd clear();

    var_4 boult(
		var_u8 word_id,
        var_u8 tm
        );

    var_4 save();

private:
    var_f4 measure_docdist(
        doc_infer* _left, 
        doc_infer* _right
        );

    // _dinfer的事件id为_eid, 调整eid对应的event_infer
    var_4 doc_into_event(
        doc_infer* _dinfer, 
        const var_u8 _eid
        );

    var_4 create_new_event(
        doc_infer* _dinfer
        );

    var_f4 calc_event_score(
        whl_node<simple_pair<doc_infer*, var_f4> >* begin, 
        whl_node<simple_pair<doc_infer*, var_f4> >* end, 
        doc_infer* _dinfer
        );

	var_4 update_key_list(
		doc_infer* _dinfer
		); 
    
	var_4 adjust_2_events(
        var_u8 _leid, 
        var_u8 _reid
        );

    var_4 load(
        var_1* _evt_path, 
        var_1* _doc_path,
        var_1* _wid2str_path
		);

public:
 	// word id-><word str, word len>
    UT_HashSearch<var_u8>       m_wid2str_map;

private:
    CP_MUTEXLOCK_RW             m_lck;

    var_1                       m_data_path[256];

    static_array<event_infer>   m_evt_infer;
    // event id->event_infer*
    UT_HashSearch<var_u8>       m_evt_indexer;
    
    static_array<doc_infer>     m_doc_infer;
    //doc id ->doc_infer*
    UT_HashSearch<var_u8>       m_doc_indexer;
   
    UT_HashSearch<var_p>        m_stat_hash;
    
    // <doc_infer->similarity> sorted by eventid
    whl_list<simple_pair<doc_infer*, var_f4> > m_cdd_docers; //m_candidaters
	
	// 
	whl_list<doc_evt_dst> 		m_cdd_evters; //m_candidaters

    //word->whl_list<doc_infer*>    //list<doc_infer*>按doc_infer::m_tm降序
    UT_HashSearch<var_u8>       m_w2doc_map;
    
    //word->whl_list<event_infer*>  //list<event_infer*>按event_infer::m_time降序
    UT_HashSearch<var_u8>       m_w2evt_map;

    //FL->event_infer
    UC_Allocator_Recycle*       m_evt_allocator;
    var_u4                      m_evt_size;

    //FL->whl_list<doc_infer*>
    UC_Allocator_Recycle*       m_dnode_allocator;        
    var_u4                      m_dnode_size;

    //FL->whl_list<event_infer*>
    UC_Allocator_Recycle*       m_enode_allocator;
    var_u4                      m_enode_size;


    //FL->sizeof(whl_node<var_vd*>)
    UC_Allocator_Recycle*       m_pointer_allocator;
    var_u4                      m_pointer_size;

	var_vd* 					m_evt_ptr;

    UT_Persistent_KeyValue<var_u8>* m_doc_storager;
};

#endif
