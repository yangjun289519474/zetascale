/*
 * File:   fdf_meta_cache.c
 * Author: Darryl Ouye
 *
 * Created on February 5, 2013
 *
 * SanDisk Proprietary Material, © Copyright 2013 SanDisk, all rights reserved.
 * http://www.sandisk.com
 *
 */
#include "utils/hashmap.h"
#include "sdf_internal.h"
#include "fdf.h"
#include "shared/container.h"
#include "cmap.h"

#define CMAP_BUCKETS	 1000

#define LOG_ID PLAT_LOG_ID_INITIAL
#define LOG_CAT PLAT_LOG_CAT_SDF_NAMING
#define LOG_TRACE PLAT_LOG_LEVEL_TRACE
#define LOG_DBG PLAT_LOG_LEVEL_DEBUG
#define LOG_DIAG PLAT_LOG_LEVEL_DIAGNOSTIC
#define LOG_INFO PLAT_LOG_LEVEL_INFO
#define LOG_ERR PLAT_LOG_LEVEL_ERROR
#define LOG_WARN PLAT_LOG_LEVEL_WARN
#define LOG_FATAL PLAT_LOG_LEVEL_FATAL

static void fdf_cmap_del_cb(void *data);

HashMap cmap_cname_hash;           // cname -> cguid
struct CMap *cmap_cguid_hash;      // cguid -> cntr_map_t

// FIXME: get rid of this and theCMC
static cntr_map_t cmc_map;

FDF_status_t fdf_cmap_init( void )
{
	// Fill in the static map for CMC
	sprintf( cmc_map.cname, "%s", CMC_PATH );
	cmc_map.cguid = CMC_CGUID;
	cmc_map.sdf_container = containerNull;
	cmc_map.size_kb = CMC_SIZE_KB;
	cmc_map.current_size = 0;
	cmc_map.num_obj = 0;
	cmc_map.state = FDF_CONTAINER_STATE_OPEN;
	cmc_map.evicting = FDF_FALSE;
	bzero( (void *) &cmc_map.enum_stats, sizeof( enum_stats_t ) );
	bzero( (void *) &cmc_map.container_stats, sizeof( FDF_container_stats_t ) );

	cmap_cguid_hash = CMapInit( CMAP_BUCKETS, MCD_MAX_NUM_CNTRS, 1, NULL, NULL, fdf_cmap_del_cb );
	cmap_cname_hash = HashMap_create( CMAP_BUCKETS, FTH_BUCKET_RW );

	if ( !cmap_cguid_hash || !cmap_cname_hash ) {
	    return FDF_FAILURE;
	} else {
		return FDF_SUCCESS;
	}
}

FDF_status_t fdf_cmap_destroy( void )
{
    CMapDestroy( cmap_cguid_hash ); 
    HashMap_destroy( cmap_cname_hash );
	return FDF_SUCCESS;
}

// We assume that the cguid - cname mapping never changes!
FDF_status_t fdf_cmap_create(
    char                    *cname,
    FDF_cguid_t              cguid,
    uint64_t                 size_kb,
    FDF_CONTAINER_STATE      state,
    FDF_boolean_t            evicting
	)
{
	char        *cname_key		= NULL;
    cntr_map_t  *cmap           = NULL;
	int			len;
    
    if ( !cname || FDF_NULL_CGUID == cguid )
        return FDF_INVALID_PARAMETER;
    
    cmap = ( cntr_map_t * ) plat_alloc( sizeof( cntr_map_t ) );

    if ( !cmap ) {
		return FDF_FAILURE_MEMORY_ALLOC;
    } else {
        sprintf( cmap->cname, "%s", cname );
        cmap->io_count = 0;
        cmap->cguid = cguid;
        cmap->sdf_container = containerNull;
        cmap->size_kb = size_kb;
        cmap->current_size = 0;
        cmap->num_obj = 0;
        cmap->state = state;
        cmap->evicting = evicting;
		cmap->read_only = FDF_FALSE;
        bzero( (void *) &cmap->enum_stats, sizeof( enum_stats_t ) );
        bzero( (void *) &cmap->container_stats, sizeof( FDF_container_stats_t ) );
	}
	
	len = strlen( cname ) + 1;
	if ( NULL == ( cname_key = (char *) plat_alloc(len))) {
	    plat_free( cmap );
		return FDF_FAILURE_MEMORY_ALLOC;
	} else {
	    strcpy( cname_key, cname );
		cname_key[len-1]= '\0';
	}

	if ( !CMapCreate( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t ), (char *) cmap, sizeof( cntr_map_t ) ) ) {
	    plat_free( cname_key );
	    plat_free( cmap );
	    plat_log_msg( 150116,
	                  LOG_CAT,
	                  LOG_DBG,
	                  "Failed to create cguid hash entry for %s - %lu", 
	                  cname, 
                      cguid
	                );
	    return FDF_FAILURE_CANNOT_CREATE_METADATA_CACHE;
	}
	    
	if ( SDF_TRUE != ( HashMap_put( cmap_cname_hash, cname_key, (void *) cguid ) ) ) {
	    plat_free( cname_key );
        CMapDelete( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t) );
	    plat_log_msg( 150117,
	                  LOG_CAT,
	                  LOG_DBG,
	                  "Failed to create cname hash entry for %s - %lu", 
	                  cname, 
                      cguid
	                );
	    return FDF_FAILURE_CANNOT_CREATE_METADATA_CACHE;
	}

	return FDF_SUCCESS;
}

FDF_status_t fdf_cmap_update(
    cntr_map_t *cmap
    )
{
    char             *cname_key         = NULL;
	FDF_status_t ret = FDF_SUCCESS;
	SDF_status_t status = SDF_SUCCESS;

	if (!cmap) {
		return FDF_FAILURE;
	}

    if ( NULL == ( cname_key = (char *) plat_alloc( strlen( cmap->cname ) + 1) ) ) {
        return FDF_FAILURE_MEMORY_ALLOC;
    }
    else {
        sprintf( cname_key, "%s", cmap->cname );
    }

    /*
     * Update mapping of cguid to cmap entry
     */
    struct CMapEntry *ptr = CMapUpdate( cmap_cguid_hash,
                        (char *) &cmap->cguid,
                        sizeof( FDF_cguid_t ),
                        (char *) cmap,
                        sizeof( cntr_map_t )
	                  );
    if (NULL == ptr) {
        plat_log_msg(160213,
                LOG_CAT,
                LOG_DBG,
                "CMapUpdate: Failed to update %s - %lu",
                cmap->cname, 
                cmap->cguid
                );
        plat_free(cname_key);
        return FDF_FAILURE;
    }

    /*
     * Update mapping of key:cname to value:cguid
     */
    status  = HashMap_put(cmap_cname_hash, (const char*)cname_key, (void *) cmap->cguid );
    if (SDF_FALSE == status) {
        plat_log_msg(160214,
                LOG_CAT, 
                LOG_DBG,
                "Entry exist: Failed to create %s - %lu, trying to replace",
                cmap->cname,
                cmap->cguid
                );

        void *old_cguid = HashMap_replace(cmap_cname_hash, (const char*)cname_key, (void *) cmap->cguid);
        if (cmap->cguid == (long int)old_cguid) {
            plat_log_msg(160215,
                    LOG_CAT,
                    LOG_DBG,
                    "Entry exist: Failed to replace %s - %lu",
                    cmap->cname,
                    cmap->cguid
                    );
            ret = FDF_FAILURE;
        }
    }
    return ret;
}


cntr_map_t *fdf_cmap_get_by_cguid(
	FDF_cguid_t cguid
    )
{
	cntr_map_t *cmap    = NULL;
	uint64_t    cmaplen = 0;
	struct CMapEntry *pme;

	if ( FDF_NULL_CGUID == cguid )
	    return NULL;

	if ( CMC_CGUID == cguid )
	    return &cmc_map;

    if ((pme = CMapGet( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t ), (char **) &cmap, &cmaplen )) != NULL ) {
		//MAx entries is same as max no. of containers, so it can not get replaced
        //CMapRelease( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t ) );
        CMapRelease_fix(pme);
	    return cmap;
	} else {
	    plat_log_msg( 150119,
	                  LOG_CAT,
	                  LOG_DBG,
	                  "%lu - not found", 
	                  cguid
	                );
	    return NULL;
	}
}

void fdf_cmap_rel(
	cntr_map_t *cmap	
    )
{

	if ( cmap == &cmc_map)
	    return;

	CMapRelease( cmap_cguid_hash, (char *) &(cmap->cguid), sizeof( FDF_cguid_t ) );
}
void fdf_cmap_rel_by_cguid(
	FDF_cguid_t cguid	
    )
{
	if ( FDF_NULL_CGUID == cguid )
	    return;

	if ( CMC_CGUID == cguid )
	    return;

	CMapRelease( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t ) );
}
FDF_cguid_t fdf_cmap_get_cguid(
    char *cname
    )
{
	FDF_cguid_t cguid    = FDF_NULL_CGUID;

	if ( !cname )
		return FDF_NULL_CGUID;

	if ( strcmp( CMC_PATH, cname ) == 0 ) {
	    return CMC_CGUID;
	}

	if (FDF_NULL_CGUID != ( cguid = ( FDF_cguid_t ) HashMap_get( cmap_cname_hash, cname ) ) ) {
	    return cguid; 
	} else {
	    plat_log_msg( 150119,
	                  LOG_CAT,
	                  LOG_DBG,
	                  "%lu - not found", 
	                  cguid
	                );
	    return FDF_NULL_CGUID;
	}
}

cntr_map_t *fdf_cmap_get_by_cname(
    char *cname
	)
{
	FDF_cguid_t cguid = FDF_NULL_CGUID;
    cntr_map_t *entry = NULL;

	if ( !cname )
	    return NULL;

	if ( strcmp( CMC_PATH, cname ) == 0 )
	    return &cmc_map;

	cguid = fdf_cmap_get_cguid(cname);

    if (FDF_NULL_CGUID != cguid) {
	    entry = fdf_cmap_get_by_cguid( cguid );
    }

    return entry;
}

static void
fdf_cmap_del_cb(void *data)
{
    cntr_map_t  *cmap           = (cntr_map_t *)data;
	plat_free(cmap);
}


FDF_status_t fdf_cmap_delete(
    FDF_cguid_t  cguid,
	char        *cname
    )
{
	int           status_cguid = 0;

	if ( FDF_NULL_CGUID == cguid || !cname )
	    return FDF_INVALID_PARAMETER;

    HashMap_remove( cmap_cname_hash, cname );

    status_cguid = CMapDelete( cmap_cguid_hash, (char *) &cguid, sizeof( FDF_cguid_t ) );


	if ( status_cguid == 1 )
	    return FDF_OBJECT_UNKNOWN;
	else
		return FDF_SUCCESS;
}

void fdf_cntr_map_destroy_map(
	cntr_map_t *cmap
    )
{
    if ( NULL != cmap ) {
        if ( cmap->cname )
            plat_free( cmap->cname );
        cmap->sdf_container = containerNull;
		plat_free( cmap );
    }
}

struct cmap_iterator *fdf_cmap_enum()
{
	struct CMapIterator *iterator = NULL;

	iterator = CMapEnum( cmap_cguid_hash );
	
	return (struct cmap_iterator *) iterator;
}

int fdf_cmap_next_enum( 
	struct cmap_iterator *iterator, 
	char **key, 
	uint32_t *keylen, 
	char **data, 
	uint64_t *datalen
	)
{
	return CMapNextEnum( cmap_cguid_hash,
	                     (struct CMapIterator *) iterator,
	                     key,
	                     keylen,
	                     data,
	                     datalen
	                   );
}

void fdf_cmap_finish_enum( 
	struct cmap_iterator *iterator 
	)
{
	CMapFinishEnum( cmap_cguid_hash, (struct CMapIterator *) iterator );
}

