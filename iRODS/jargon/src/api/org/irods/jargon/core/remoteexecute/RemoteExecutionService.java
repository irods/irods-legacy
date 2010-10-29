package org.irods.jargon.core.remoteexecute;

import java.io.InputStream;

import org.irods.jargon.core.exception.JargonException;

public interface RemoteExecutionService {

	public abstract InputStream execute() throws JargonException;

}