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
//  Method.java  -  edu.sdsc.grid.io.irods.Method
//
//  CLASS HIERARCHY
//  java.lang.Object
//      |
//      +-edu.sdsc.grid.io.irods.Method
//
//
//  PRINCIPAL AUTHOR
//  Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.io.irods;

import java.util.Vector;

/**
 * a microservice
 */
class Method extends Parameter {
	String methodName;
	Parameter[] parameters;

	Method(String methodName, Parameter[] parameters) {
		this.parameters = parameters;
		this.methodName = methodName;
	}

	Parameter[] getParameters() {
		if (parameters != null)
			return parameters;

		return null;
	}

	Object getParameter(int index) {
		if (parameters != null)
			return parameters[index];

		return null;
	}

	/**
	 * Return only those parameters which already have values. Also includes
	 * subparameters of submethods. For use by the rule engine as input
	 * parameters
	 */
	Parameter[] getValuedParameters() {
		if (parameters != null) {
			Vector<Parameter> temp = new Vector(parameters.length);

			for (Parameter p : parameters) {
				if (p instanceof Method) {
					for (Parameter q : ((Method) p).getValuedParameters()) {
						temp.add(q);
					}
				} else if (p.getType() != p.NULL_PI) {
					temp.add(p);
				}
			}
			return temp.toArray(parameters);
		}

		return null;
	}

	String getMethodName() {
		return methodName;
	}

	public String toString() {
		String temp = methodName + "(";
		for (Parameter p : parameters) {
			if (p instanceof Method) {
				temp += p.toString() + ",";
			} else
				temp += p.getUniqueName() + ",";
		}
		// get rid of the last ',' add a ')'
		temp = temp.substring(0, temp.length() - 1) + ")";

		return temp;
	}
}
