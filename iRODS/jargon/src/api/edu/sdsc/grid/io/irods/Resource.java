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

import org.irods.jargon.core.exception.DuplicateDataException;
import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

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

}
