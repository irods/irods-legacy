#!/bin/bash -x

# For iRODS 3.3, the patches (updates) for the ICAT are all new
# specific queries, so instead of updates at the DBMS level this is
# done via the following iadmin commands.  To make the patch, start up
# irods 3.3, authenticate as the admin (iinit), and then run this
# script (./patch3.2to3.3.sh) to add these new specific queries.
#
# These new specific queries are used by Jargon for much better
# performance in instances with very large sets of ACLs and
# users.  Thanks to the iPlant development team for these
# improvements.

iadmin asq "select alias,sqlStr from R_SPECIFIC_QUERY where alias like ?" listQueryByAliasLike

iadmin asq "select alias,sqlStr from R_SPECIFIC_QUERY where alias = ?" findQueryByAlias

iadmin asq "SELECT c.parent_coll_name, c.coll_name, c.create_ts, c.modify_ts, c.coll_id, c.coll_owner_name, c.coll_owner_zone,c.coll_type, u.user_name, u.zone_name, a.access_type_id, u.user_id FROM r_coll_main c JOIN r_objt_access a ON c.coll_id = a.object_id JOIN r_user_main u ON a.user_id = u.user_id WHERE c.parent_coll_name = ? LIMIT ? OFFSET ?" ilsLACollections

iadmin asq "SELECT s.coll_name, s.data_name, s.create_ts, s.modify_ts, s.data_id, s.data_size, s.data_repl_num, s.data_owner_name, s.data_owner_zone, u.user_name, u.user_id, a.access_type_id,  u.user_type_name, u.zone_name FROM ( SELECT c.coll_name, d.data_name, d.create_ts, d.modify_ts, d.data_id, d.data_repl_num, d.data_size, d.data_owner_name, d.data_owner_zone FROM r_coll_main c JOIN r_data_main d ON c.coll_id = d.coll_id  WHERE c.coll_name = ?  ORDER BY d.data_name) s JOIN r_objt_access a ON s.data_id = a.object_id JOIN r_user_main u ON a.user_id = u.user_id LIMIT ? OFFSET ?" ilsLADataObjects

iadmin asq "SELECT DISTINCT r_coll_main.coll_id, r_coll_main.parent_coll_name, r_coll_main.coll_name, r_coll_main.coll_owner_name, r_coll_main.coll_owner_zone, r_meta_main.meta_attr_name, r_meta_main.meta_attr_value, r_meta_main.meta_attr_unit FROM r_coll_main JOIN r_objt_metamap ON r_coll_main.coll_id = r_objt_metamap.object_id JOIN r_meta_main ON r_objt_metamap.meta_id = r_meta_main.meta_id WHERE r_meta_main.meta_attr_unit = 'iRODSUserTagging:Share' AND r_coll_main.coll_owner_name = ? AND r_coll_main.coll_owner_zone = ? ORDER BY r_coll_main.parent_coll_name ASC, r_coll_main.coll_name ASC" listSharedCollectionsOwnedByUser

iadmin asq "SELECT DISTINCT r_coll_main.coll_id, r_coll_main.parent_coll_name, r_coll_main.coll_name, r_coll_main.coll_owner_name, r_coll_main.coll_owner_zone, r_meta_main.meta_attr_name, r_meta_main.meta_attr_value, r_meta_main.meta_attr_unit, r_user_main.user_name, r_user_main.zone_name, r_objt_access.access_type_id FROM r_coll_main JOIN r_objt_metamap ON r_coll_main.coll_id = r_objt_metamap.object_id JOIN r_meta_main ON r_objt_metamap.meta_id = r_meta_main.meta_id JOIN r_objt_access ON r_coll_main.coll_id = r_objt_access.object_id JOIN r_user_main ON r_objt_access.user_id = r_user_main.user_id WHERE r_meta_main.meta_attr_unit = 'iRODSUserTagging:Share' AND r_user_main.user_name = ? AND r_user_main.zone_name = ? AND r_coll_main.coll_owner_name <> ? ORDER BY r_coll_main.parent_coll_name ASC, r_coll_main.coll_name ASC" listSharedCollectionsSharedWithUser

#The following commands, if uncommented, would reverse the above (asq is add specific query, rsq is remove specific query):
#iadmin rsq listQueryByAliasLike
#iadmin rsq findQueryByAlias
#iadmin rsq ilsLACollections
#iadmin rsq ilsLADataObjects
#iadmin rsq listSharedCollectionsOwnedByUser
#iadmin rsq listSharedCollectionsSharedWithUser

