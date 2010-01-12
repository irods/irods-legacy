//
//  Copyright (c) 2008  San Diego Supercomputer Center (SDSC),
//  University of California, San Diego (UCSD), San Diego, CA, USA.
//
//  Users and possessors of this source code are hereby granted a
//  nonexclusive, royalty-free copyright and design patent license
//  to use this code in individual software.  License is not granted
//  for commercial resale, in whole or in part, without prior written
//  permission from SDSC/UCSD.  This source is provided "AS IS"
//  without express or implied warranty of any kind.
//
//
//  FILE
//  Rule.java  -  edu.sdsc.grid.io.irods.Rule
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.Rule
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.io.IOException;
import java.io.InputStream;
import java.util.StringTokenizer;
import java.util.Vector;

import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.local.LocalFile;

/**
 * Java object for an IRODS rule. see https://www.irods.org/index.php/Rules
 *<P>
 * 
 * @author Lucas Gilbert
 * @since JARGON2.0
 */
class Rule {

	static final String INT_PI = "INT_PI";
	static final String myInt = "myInt";
	static final String STR_PI = "STR_PI";
	static final String myStr = "myStr";
	static final String BUF_LEN_PI = "BUF_LEN_PI";
	static final String BinBytesBuf_PI = "BinBytesBuf_PI";
	static final String ExecCmdOut_PI = "ExecCmdOut_PI";
	static final String DataObjInp_PI = "DataObjInp_PI";

	static final String CL_PUT_ACTION = "CL_PUT_ACTION";
	static final String CL_GET_ACTION = "CL_GET_ACTION";

	static final String buflen = "buflen";
	static final String buf = "buf";

	private static final int RULE_SIZE_BUFFER = 64000;

	String label;

	String type;

	/**
	 * Could be String, int, byte array or more?
	 */
	Object parameter;

	String ruleName;
	Method[] methods;
	Parameter[] outputs;

	Parameter[] inputs;

	/**
	 * @deprecated This constructor initializes unread fields
	 * @param label
	 * @param type
	 * @param parameter
	 */
	public Rule(String label, String type, Object parameter) {

		this.label = label;
		this.type = type;
		this.parameter = parameter;
	}

	/**
	 * @deprecated This constructor initializes unread fields
	 * @param ruleName
	 * @param microservices
	 * @param outputs
	 */
	public Rule(String ruleName, Method[] microservices, Parameter[] outputs) {
		this.ruleName = ruleName;
		methods = microservices;
		this.outputs = outputs;
	}

	Rule(String ruleName, Parameter[] inputs, Parameter[] outputs) {
		this.ruleName = ruleName;
		this.inputs = inputs;
		this.outputs = outputs;
	}

	/**
	 * @return the parameter value of the parameter tag. Other values, like
	 *         buffer length, can be derived from it, if the type is known.
	 */
	static Object getParameter(final String type, final Tag parameterTag) {
		if (type.equals(INT_PI)) {
			return parameterTag.getTag(INT_PI).getTag(myInt).getIntValue() + "";
		} else if (type.equals(BUF_LEN_PI)) {
			return parameterTag.getTag(BinBytesBuf_PI).getTag(buf)
					.getStringValue();
		} else if (type.equals(ExecCmdOut_PI)) {
			Tag exec = parameterTag.getTag(ExecCmdOut_PI);

			// last tag is status
			int length = exec.getLength() - 1;
			String[] results = new String[length];
			for (int i = 0; i < length; i++) {
				if (exec.getTag(BinBytesBuf_PI, i).getTag(buflen).getIntValue() > 0) {
					results[i] = exec.getTag(BinBytesBuf_PI, i).getTag(buf)
							.getStringValue();
				}
			}
			return results;
		} else if (type.equals(STR_PI)) {
			return parameterTag.getTag(STR_PI).getTag(myStr).getStringValue();
		} else {
			return parameterTag.getTag(type);
		}
	}

	static Parameter[] executeRule(final IRODSFileSystem irodsFileSystem,
			String rule) throws IOException {
		Vector<Parameter> inputs = new Vector<Parameter>();
		Vector<Parameter> outputs = new Vector<Parameter>();

		StringTokenizer tokens = new StringTokenizer(rule, "\n");

		String ruleBody = processRuleBody(tokens);

		// if formatting error, such as only one line, below breaks
		if (!tokens.hasMoreTokens())
			throw new IllegalArgumentException("Rule stream is malformed");

		// process the rule attributes
		processRuleAttributesLine(tokens, inputs);

		// if formatting error, such as only one line, below breaks
		if (!tokens.hasMoreTokens())
			throw new IllegalArgumentException("Rule stream is malformed");

		processRuleOutputLine(tokens, outputs);

		return Rule.readResult(irodsFileSystem, irodsFileSystem.commands
				.executeRule(ruleBody, inputs.toArray(new Parameter[0]),
						outputs.toArray(new Parameter[0])));

	}

/**
	 * ?to be deprecated {@see edu.sdsc.grid.io.irods.IRODSCommands#executeRule(String, Parameter[], Parameter[]) 
	 * *need a cleaner way to do this
	 * *class visibility issues
	 * *use of deprecated StringBufferInputStream?  just take a string as a param
	 * @param fileSystem
	 * @param ruleStream
	 * @return
	 * @throws IOException
	 */
	static Parameter[] executeRule(final IRODSFileSystem fileSystem,
			InputStream ruleStream) throws IOException {
		StringBuilder rule = new StringBuilder();
		// Probably should just read the whole thing in one buffer, but what
		// size?
		byte[] b = new byte[RULE_SIZE_BUFFER];
		int read;

		while ((read = ruleStream.read(b)) != -1) {
			rule.append(new String(b, 0, read));
		}

		return executeRule(fileSystem, rule.toString());
	}

	/**
	 * @param tokens
	 * @return
	 */
	static String processRuleBody(final StringTokenizer tokens) {
		String total;
		// if formatting error, such as only one line, below breaks
		if (!tokens.hasMoreTokens())
			throw new IllegalArgumentException("Rule stream is malformed");

		// Remove comments
		total = tokens.nextToken();
		while (total.startsWith("#")) {
			total = tokens.nextToken();
		}
		// find the rule
		return total;
	}

	/**
	 * @param tokens
	 * @param outputs
	 */
	static void processRuleOutputLine(final StringTokenizer tokens,
			Vector<Parameter> outputs) {
		String ruleOutputLine;
		int index;
		// find the outputs
		ruleOutputLine = tokens.nextToken();
		index = ruleOutputLine.indexOf('%');
		while (index >= 0) {
			outputs.add(new Parameter(ruleOutputLine.substring(0, index), null,
					null));
			ruleOutputLine = ruleOutputLine.substring(index + 1);
			index = ruleOutputLine.indexOf("%");
		}
		// add the final one
		outputs.add(new Parameter(ruleOutputLine, null, null));
	}

	/**
	 * @param tokens
	 * @param inputs
	 */
	static void processRuleAttributesLine(final StringTokenizer tokens,
			Vector<Parameter> inputs) {
		String attribLine;
		int index;
		int index2;
		// find the labels
		attribLine = tokens.nextToken();
		// looking for parameters on second line of rule, separated by %
		// there may not be one if only 'null' on second line
		if (attribLine.trim().equals("null")) {
			// do what? need to add a null param
			inputs.add(new Parameter());
		} else {

			index = attribLine.indexOf('%');
			while (index >= 0) {
				index2 = attribLine.indexOf('=');
				if (index2 < 0)
					throw new IllegalArgumentException(
							"Rule stream is malformed");
				inputs.add(new Parameter(attribLine.substring(0, index2),
						attribLine.substring(index2 + 1, attribLine.indexOf(
								'%', index2))));
				attribLine = attribLine.substring(index + 1);
				index = attribLine.indexOf("%");
			}
			index2 = attribLine.indexOf('=');
			if (index2 < 0)
				throw new IllegalArgumentException("Rule stream is malformed");
			// add the final one
			inputs.add(new Parameter(attribLine.substring(0, index2),
					attribLine.substring(index2 + 1)));
		}
	}

	/**
	 * Process the <code>Tag</code> that is returned upon invocation of an IRODS
	 * Rule.
	 * 
	 * @param fileSystem
	 *            {@link edu.sdsc.grid.io.irods.IRODSFileSystem IRODSFileSystem}
	 *            used to process any client requests that result from the rule
	 *            invocation.
	 * @param rulesTag
	 *            {@link edu.sdsc.grid.io.irods.Tag Tag} that results from the
	 *            invocation of a rule
	 * @return {@link edu.sdsc.grid.io.irods.Parameter Parameter[]} that is a
	 *         translation of the <code>Tag</code> produced by the rule
	 *         invocation. Note that the method in the past returned null, but
	 *         now will return an empty <code>Parameter[]</code>.
	 * @throws IOException
	 */
	static Parameter[] readResult(IRODSFileSystem fileSystem, final Tag rulesTag)
			throws IOException {

		Parameter[] result;
		if (rulesTag == null) {
			result = new Parameter[0];
		} else {
			result = processNonNullRuleResult(fileSystem, rulesTag);
		}
		return result;
	}

	/**
	 * Take the raw output of the rule and process into a set of Parameter
	 * objects.
	 * 
	 * @param fileSystem
	 * @param rulesTag
	 *            {@link edu.sdsc.grid.io.irods.Tag Tag} containing the response
	 *            from IRODS for this rule execution
	 * @return {@link edu.sdsc.grid.io.irods.Parameter Parameter} containing the
	 *         parsed-out response of the <code>Tag</code> that came back from
	 *         IRODS
	 * @throws IOException
	 */
	private static Parameter[] processNonNullRuleResult(
			final IRODSFileSystem fileSystem, final Tag rulesTag)
			throws IOException {
		int parametersLength = rulesTag.getTag(IRODSConstants.paramLen)
				.getIntValue();

		// for client action has TagName "MsParam_PI, value "CL_PUT_ACTION"

		// for GenQOut has MsParam_PI, value GenQOut

		if (parametersLength > 0) {
			String label = null;
			String type = null;
			Object value = null;
			Parameter[] parameters = new Parameter[parametersLength];

			for (int i = 0; i < parametersLength; i++) {
				Tag msParam = rulesTag.getTag(IRODSConstants.MsParam_PI, i);
				label = msParam.getTag(IRODSConstants.label).getStringValue();
				type = msParam.getTag(IRODSConstants.type).getStringValue();
				value = getParameter(type, msParam);
				// need to differentiate tag that is client request from
				// tag that is an array
				if (label.equals("CL_PUT_ACTION")
						|| label.equals("CL_GET_ACTION")) {

					value = processRuleResponseTag(fileSystem, label, type,
							value, msParam);

				}
				parameters[i] = new Parameter(label, value, type);
			}

			fileSystem.commands.operationComplete(0);
			return parameters;
		}

		return new Parameter[0];
	}

	/**
	 * @param fileSystem
	 * @param label
	 * @param type
	 * @param value
	 * @param msParam
	 * @return
	 * @throws IOException
	 * 
	 *             retained note on this method... should check intInfo if ==
	 *             SYS_SVR_TO_CLI_MSI_REQUEST 99999995
	 *             /*lib/core/include/rodsDef.h
	 * 
	 *             // this is the return value for the rcExecMyRule call
	 *             indicating the // server is requesting the client to client
	 *             to perform certain task #define SYS_SVR_TO_CLI_MSI_REQUEST
	 *             99999995 #define SYS_SVR_TO_CLI_COLL_STAT 99999996 #define
	 *             SYS_CLI_TO_SVR_COLL_STAT_REPLY 99999997
	 * 
	 *             // definition for iRods server to client action request from
	 *             a microservice. // these definitions are put in the "label"
	 *             field of MsParam
	 * 
	 *             #define CL_PUT_ACTION "CL_PUT_ACTION" #define CL_GET_ACTION
	 *             "CL_GET_ACTION" #define CL_ZONE_OPR_INX "CL_ZONE_OPR_INX"
	 */

	protected static Object processRuleResponseTag(
			final IRODSFileSystem fileSystem, final String label,
			final String type, Object value, Tag msParam) throws IOException {
		{

			// a tag may be returned from a rule containing an array,
			// e.g. exec gen query.
			// determine if this is indeed a client request. Gets NPE when
			// trying to construct the IRODSFile below

			// server is requesting client action
			Tag fileAction = msParam.getTag(type);

			IRODSFile irodsFile = new IRODSFile(fileSystem, fileAction.getTag(
					"objPath").getStringValue());

			String otherFilePath = fileAction.getTag("KeyValPair_PI").getTag(
					"svalue").getStringValue();
			String otherFileType = fileAction.getTag("KeyValPair_PI").getTag(
					"keyWord").getStringValue();
			GeneralFile otherFile = null;
			if (otherFileType.equals("localPath")) {
				otherFile = new LocalFile(otherFilePath);
			} else {
				throw new IllegalArgumentException(
						"Rule requests tranfer from unknown protocol");
			}

			if (label.equals(CL_GET_ACTION)) {
				if (otherFile.exists()) {
					throw new IOException(otherFile + " already exists");
				}
				fileSystem.commands.get(irodsFile, otherFile);
				value = otherFile;
			} else if (label.equals(CL_PUT_ACTION)) {
				fileSystem.commands.put(otherFile, irodsFile, false);
				value = irodsFile;
			} else {
				throw new UnsupportedOperationException(
						"Rule requests unknown action");
			}
		}
		return value;
	}
}