/**
 *   Copyright (c) Oct 9, 2008  DICE Research,
 *   University of California, San Diego (UCSD), San Diego, CA, USA.
 *
 *   Users and possessors of this source code are hereby granted a
 *   nonexclusive, royalty-free copyright and design patent license
 *   to use this code in individual software.  License is not granted
 *   for commercial resale, in whole or in part, without prior written
 *   permission from SDSC/UCSD.  This source is provided "AS IS"
 *   without express or implied warranty of any kind.
 *
 *
 *   IRODSAdmin
 *   Resource.java  -  edu.sdsc.grid.io.irods.Resource
 *
 *   CLASS HIERARCHY
 *   java.lang.Object
 *       |
 *       +-edu.sdsc.grid.io.irods.Domain
 *          |
 *          +-edu.sdsc.grid.io.irods.Resource
 *
 *
 *   PRINCIPAL AUTHOR
 *   Lucas Gilbert, SDSC/UCSD
 */

package edu.sdsc.grid.io.irods;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.accessobject.IRODSAccessObjectFactory;
import org.irods.jargon.core.accessobject.IRODSAccessObjectFactoryImpl;
import org.irods.jargon.core.accessobject.IRODSGenQueryExecutor;
import org.irods.jargon.core.exception.DuplicateDataException;
import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.pub.domain.AvuData;
import org.irods.jargon.core.query.AVUQueryElement;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.JargonQueryException;
import org.irods.jargon.core.query.MetaDataAndDomainData;
import org.irods.jargon.core.query.RodsGenQueryEnum;
import org.irods.jargon.core.query.MetaDataAndDomainData.MetadataDomain;
import org.irods.jargon.core.utils.AccessObjectQueryProcessingUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.MetaDataCondition;
import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;
import edu.sdsc.grid.io.Namespace;
import edu.sdsc.grid.io.ResourceMetaData;

/**
 * This is a transitional service to access/modify resource information. Later
 * versions of the Jargon API will see this class refactored, though all efforts
 * will be made to preserve the current method signatures.
 * 
 * @author iktome
 */
public class Resource extends Domain {

	private Logger log = LoggerFactory.getLogger(Resource.class);

	static final String iName = "resource";

	private static final Object COMMA = ", ";

	private static final Object WHERE = " WHERE ";

	private static final Object EQUALS_AND_QUOTE = " = '";

	public Resource(IRODSFileSystem irodsFileSystem) {
		super(irodsFileSystem, "resource", "resc_type", "r_resc_main");
	}

	/**
	 * Queries the fileSystem to aqcuire all the values for this domain. So the
	 * user domain returns all the users.
	 * 
	 * @return
	 */
	public String[] listSubjects() throws IOException {
		return irodsFileSystem.commands.simpleQuery(
				"select resc_name from r_resc_main", null);
	}

	// mkresc Name Type Class Host Path (make Resource)
	public void addResource(String resourceName, String resourceType,
			String resourceClass, String host, String vaultFilePath)
			throws IOException {
		String[] args = { "add", name, resourceName, resourceType.toString(),
				resourceClass, host, vaultFilePath };
		irodsFileSystem.commands.admin(args);
	}

	// rmresc Name (remove resource)
	public void deleteResource(String resourceName) throws IOException {
		String[] args = { "rm", name, resourceName };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyClass(String resourceName, String newClass)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "class", newClass };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyHost(String resourceName, String newHost)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "host", newHost };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyPath(String resourceName, String newPath)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "path", newPath };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyComment(String resourceName, String newComment)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "comment", newComment };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyInfo(String resourceName, String newInfo)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "info", newInfo };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyFreespace(String resourceName, String newValue)
			throws IOException {
		String[] args = { "modify", iName, resourceName, "type", newValue };
		irodsFileSystem.commands.admin(args);
	}

	public List<String> listResourcesInResourceGroup(String resourceGroupName)
			throws JargonException {
		if (log.isInfoEnabled()) {
			log
					.info("listing resources in resource group:"
							+ resourceGroupName);
		}

		List<String> resources = new ArrayList<String>();

		final MetaDataCondition[] condition = { MetaDataSet.newCondition(
				ResourceMetaData.RESOURCE_GROUP, MetaDataCondition.EQUAL,
				resourceGroupName) };
		final MetaDataSelect[] select = { MetaDataSet
				.newSelection(ResourceMetaData.RESOURCE_NAME)

		};
		MetaDataRecordList[] results;
		try {
			results = irodsFileSystem.commands.query(condition, select, 500,
					Namespace.RESOURCE);
		} catch (IOException e) {
			String msg = "IOException in query";
			log.error("IOException in query", e);
			throw new JargonException(msg, e);
		}
		int i = 0;

		for (final MetaDataRecordList result : results) {
			final String resource = result.getStringValue(i++);
			if ((resource != null) && (resource.length() > 0)) {
				resources.add(resource);
			}
		}

		return resources;
	}

	/**
	 * List the resource groups in IRODS
	 * 
	 * @return <code>List<String></code> containing the resource groups
	 * @throws JargonException
	 */
	public List<String> listResourceGroups() throws JargonException {
		log.info("listing resource groups");
		List<String> resourceGroups = new ArrayList<String>();
		String rgQuery = "select distinct resc_group_name from r_resc_group";

		String[] queryResponse;
		try {
			queryResponse = irodsFileSystem.commands.simpleQuery(rgQuery, null);
			if (log.isInfoEnabled()) {
				log.info("resource groups found:");

			}
			// null means no record found, return an empty list
			if (queryResponse == null) {
				return resourceGroups;
			}

			for (int i = 0; i < queryResponse.length; i++) {
				if (log.isInfoEnabled()) {
					log.info("    resource group:" + queryResponse[i]);
				}
				resourceGroups.add((queryResponse[i]).trim());
			}
		} catch (IRODSException ie) {
			log.error("IRODS exception sending iadmin command, irods code="
					+ ie.getType(), ie);
			throw new JargonException(
					"IRODS exception sending iadmin lrg command, irods code ="
							+ ie.getType(), ie);
		} catch (IOException e) {
			log.error("IO exception sending iadmin command", e);
			throw new JargonException(
					"IO exception sending iadmin lrg command", e);
		}

		return resourceGroups;
	}

	/**
	 * Add a resource to a resource group.
	 * 
	 * @param resourceName
	 *            <code>String</code> containing the name of the resource to add
	 *            to the specified group.
	 * @param resourceGroupName
	 *            <code>String<code> containing the name of the resource group to which the resource will be added.
	 * @throws JargonException
	 *             exception that indicates some sort of error condition
	 * @throws DuplicateDataException
	 *             exception that indicates an attempt to add duplicate data.
	 *             Generally, this should be treated as a warning, versus
	 *             returned as an error.
	 */
	public void addResourceToResourceGroup(String resourceName,
			String resourceGroupName) throws JargonException,
			DuplicateDataException {

		if (resourceName == null || resourceName.length() == 0) {
			throw new JargonException("resource name cannot be null or blank");
		}

		if (resourceGroupName == null || resourceGroupName.length() == 0) {
			throw new JargonException(
					"resource group name cannot be null or blank");
		}

		/*
		 * equivilent of iadmin atrg command generalAdmin(0, "modify",
		 * "resourcegroup", cmdToken[1], "add", cmdToken[2], "", "", "");
		 */
		if (log.isInfoEnabled()) {
			log.info("adding resource group:" + resourceGroupName
					+ " to resource:" + resourceName);
		}

		String[] args = { "modify", "resourcegroup", resourceGroupName, "add",
				resourceName };
		try {
			irodsFileSystem.commands.admin(args);
		} catch (IRODSException ie) {
			// note that an -809000 exception means the resource group was
			// already there, treat as a
			// non-fatal condition
			if (ie.getType() == -809000) {
				log
						.warn("adding a duplicate resource to resource group, resource="
								+ resourceName
								+ " resource group="
								+ resourceGroupName);
				throw new DuplicateDataException(
						"duplicate resource added to resource group");
			}

			log.error("IRODS exception sending iadmin command, irods code="
					+ ie.getType(), ie);
			throw new JargonException(
					"IRODS exception sending iadmin atrg command, irods code ="
							+ ie.getType(), ie);
		} catch (IOException e) {
			log.error("IO exception sending iadmin command", e);
			throw new JargonException(
					"IO exception sending iadmin atrg command", e);
		}
		log.info("successfully added resource group");
	}

	/**
	 * Remove the resource from the given resource group. Note that, if a
	 * resource does not exist in the group, the condition will be silently
	 * ignored.
	 * 
	 * @param resourceName
	 *            <code>String</code> giving the name of the resource to remove
	 *            from the resource group.
	 * @param resourceGroupName
	 *            <code>String</code> giving the name of the resource group from
	 *            which the resource will be removed.
	 * @throws JargonException
	 *             an error condition caused by this call to IRODS
	 */
	public void removeResourceFromResourceGroup(String resourceName,
			String resourceGroupName) throws JargonException {

		if (resourceName == null || resourceName.length() == 0) {
			throw new JargonException("resource name cannot be null or blank");
		}

		if (resourceGroupName == null || resourceGroupName.length() == 0) {
			throw new JargonException(
					"resource group name cannot be null or blank");
		}

		if (log.isInfoEnabled()) {
			log.info("removing resource group:" + resourceGroupName);
		}

		/*
		 * equiv of rmrg generalAdmin(0, "modify", "resourcegroup", cmdToken[1],
		 * "remove", cmdToken[2], "", "", "");
		 */

		String[] args = { "modify", "resourcegroup", resourceGroupName,
				"remove", resourceName };
		try {
			irodsFileSystem.commands.admin(args);
		} catch (IRODSException ie) {
			if (ie.getMessage().indexOf("-831000") > -1) {
				log.warn("resource does not exist, remove will silenly ignore");
			} else {
				log.error("IRODS exception sending iadmin command, irods code="
						+ ie.getType(), ie);
				throw new JargonException(
						"IRODS exception sending iadmin rmrg command, irods code ="
								+ ie.getType(), ie);
			}
		} catch (IOException e) {
			log.error("IO exception sending iadmin command", e);
			throw new JargonException(
					"IO exception sending iadmin rmrg command", e);
		}

		log.info("successfully removed resource from group");

	}

	/**
	 * Add an AVU value for a resource
	 * 
	 * @param resourceName
	 *            <code>String</code> with the name of the resource
	 * @param values
	 *            <code>String[]</code> containing an AVU in the form (attrib
	 *            name, attrib value) or (attrib name, attrib value, attrib
	 *            units)
	 * @throws JargonException
	 */
	public void addMetadataToResource(final String resourceName,
			final String[] values) throws JargonException {

		if (resourceName == null || resourceName.length() == 0) {
			throw new JargonException("null or empty resourceName");
		}

		if (values == null) {
			throw new JargonException("null metadata values");
		}

		// other validity checks done in IRODSCommands

		log.info("adding AVU metadata to resource:{}", resourceName);
		log.info("metadata values:{}", values);

		try {
			irodsFileSystem.commands.modifyResourceMetaData(resourceName,
					values);
		} catch (IRODSException ie) {

			log.error("IRODS exception , irods code=" + ie.getType(), ie);
			throw new JargonException("IRODS exception , irods code ="
					+ ie.getType(), ie);

		} catch (IOException e) {
			log.error("IO exception sending command", e);
			throw new JargonException("IO exception sending command", e);
		}

		log.info("avu metadata added");

	}

	/**
	 * Delete the given AVU triple from the metadata associated with a Resource.
	 * 
	 * @param resourceName
	 *            <code>String</code> with the name of the resource.
	 * @param values
	 *            <code>String[]</code> with an attribute/value, or
	 *            attribute/value/units triple to be deleted from the resource.
	 * @throws JargonException
	 */
	public void deleteMetadataFromResource(final String resourceName,
			final String[] values) throws JargonException {
		if (resourceName == null || resourceName.length() == 0) {
			throw new JargonException("null or empty resourceName");
		}

		if (values == null) {
			throw new JargonException("null metadata values");
		}

		// other validity checks done in IRODSCommands

		log.info("deleting AVU metadata from resource:{}", resourceName);
		log.info("metadata values:{}", values);

		try {
			irodsFileSystem.commands.deleteResourceMetaData(resourceName,
					values);
		} catch (IRODSException ie) {

			log.error("IRODS exception , irods code=" + ie.getType(), ie);
			throw new JargonException("IRODS exception , irods code ="
					+ ie.getType(), ie);

		} catch (IOException e) {
			log.error("IO exception sending command", e);
			throw new JargonException("IO exception sending command", e);
		}

		log.info("avu metadata deleted");

	}

	/**
	 * List the AVU metadata associated with a given iRODS Resource.
	 * @param resourceName <code>String</code> with the name of an iRODS resource.
	 * @return <code>List</code> of {@link org.irods.jargon.core.query.AvuData} with the 
	 * metadata for the given iRODS Resource.
	 * @throws JargonException
	 */
	public List<AvuData> listResourceMetadata(final String resourceName)
			throws JargonException {
		if (resourceName == null || resourceName.isEmpty()) {
			throw new JargonException("null or empty resourceName");
		}
		log.info("list resource metadata for {}", resourceName);

		final StringBuilder sb = new StringBuilder();
		sb.append("SELECT ");
		sb.append(RodsGenQueryEnum.COL_META_RESC_ATTR_NAME.getName());
		sb.append(COMMA);
		sb.append(RodsGenQueryEnum.COL_META_RESC_ATTR_VALUE.getName());
		sb.append(COMMA);
		sb.append(RodsGenQueryEnum.COL_META_RESC_ATTR_UNITS.getName());
		sb.append(WHERE);
		sb.append(RodsGenQueryEnum.COL_R_RESC_NAME.getName());
		sb.append(EQUALS_AND_QUOTE);
		sb.append(resourceName);
		sb.append("'");
		log.debug("resource avu list query: {}", sb.toString());
		final IRODSQuery irodsQuery = IRODSQuery.instance(sb.toString(), 5000);

		IRODSAccessObjectFactory irodsAccessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());

		final IRODSGenQueryExecutor irodsGenQueryExecutor = irodsAccessObjectFactory
				.getIRODSGenQueryExcecutor();

		IRODSQueryResultSet resultSet;

		try {
			resultSet = irodsGenQueryExecutor.executeIRODSQuery(irodsQuery, 0);
		} catch (JargonQueryException e) {
			log
					.error("query exception for resource query: "
							+ sb.toString(), e);
			throw new JargonException("error in resource query");
		}

		return AccessObjectQueryProcessingUtils
				.buildAvuDataListFromResultSet(resultSet);

	}

}
