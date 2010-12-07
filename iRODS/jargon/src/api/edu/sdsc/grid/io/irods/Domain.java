package edu.sdsc.grid.io.irods;

import java.io.IOException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Query and manipulate ICAT tokens, kept in the tabel <code>r_tokn_main</code>.
 * This class should be considered obsolete and subject to removal and revamping.
 * 
 * @author iktome
 */
public class Domain {
	protected final String name;
	protected final String typeName;
	protected final String tableName;
	protected final IRODSFileSystem irodsFileSystem;
	
	private  Logger log = LoggerFactory.getLogger(this.getClass());


	Domain(IRODSFileSystem irodsFileSystem, String name, String typeName,
			String tableName) {
		this.name = name;
		this.typeName = typeName;
		this.tableName = tableName;
		this.irodsFileSystem = irodsFileSystem;
	}

	String getName() {
		return name;
	}

	String getTypeName() {
		return typeName;
	}

	String getTableName() {
		return tableName;
	}

	@Override
	public String toString() {
		return name;
	}

	/**
	 * Queries the fileSystem to acquire all the types for this domain.
	 * 
	 * @return
	 */
	public String[] listTypes() throws IOException {
		return irodsFileSystem.commands.simpleQuery(
				"select token_name from r_tokn_main where token_namespace = ?",
				typeName);
	}

	public void addType(String newType) throws IOException {
		addType(newType, "null", "null", "null");
	}

	// at tokenNamespace Name Value [Value2] [Value3] (add token)
	public void addType(String newType, String value, String value2,
			String value3) throws IOException {
		String[] args = { "add", "token", getTypeName(), newType, value,
				value2, value3 };
		irodsFileSystem.commands.admin(args);
	}

	// rt tokenNamespace Name Value (remove token)
	public void deleteType(String newType) throws IOException {
		String[] args = { "rm", "token", getTypeName(), newType };
		irodsFileSystem.commands.admin(args);
	}

	public void modifyType(String subjectName, String newType)
			throws IOException {
		String[] args = { "modify", name, subjectName, "type", newType };
		irodsFileSystem.commands.admin(args);
	}
}