package edu.sdsc.grid.io.irods;

import static org.irods.jargon.core.connection.ConnectionConstants.INT_LENGTH;

import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.net.ProtocolException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.channels.ClosedChannelException;
import java.util.Date;

import org.irods.jargon.core.connection.IRODSManagedConnection;
import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.GeneralFileSystem;
import edu.sdsc.grid.io.GeneralRandomAccessFile;
import edu.sdsc.grid.io.Host;

/**
 * Wraps a connection to the irods server desribed by the given IRODSAccount.
 * This implmementation will open and close the underlying connection directly
 * (the socket will actually be closed when done).
 * 
 * Jargon services do not directly access the <code>IRODSConnection</code>,
 * rather, they use the {@link IRODSCommand IRODSCommand} interface. The methods
 * in this class are not synchronized, instead, the <code>IRODSCommand</code>
 * class will do all necessary synchronization as messages are sent and received
 * from IRODS.
 * 
 * This is a transitional refactoring, and will change in the future.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 *         note: eventually switch to default visibiity so no methods are public
 * 
 */
public final class IRODSConnection implements IRODSManagedConnection {

	// private IRODSServerProperties irodsServerProperties;
	private Logger log = LoggerFactory.getLogger(IRODSConnection.class);
	final IRODSAccount irodsAccount;
	// private final IRODSProtocolManager irodsProtocolManager;
	private String connectionInternalIdentifier;
	private boolean connected = false;
	private Socket connection;

	public Socket getConnection() {
		return connection;
	}

	private InputStream irodsInputStream;

	public InputStream getIrodsInputStream() {
		return irodsInputStream;
	}

	public OutputStream getIrodsOutputStream() {
		return irodsOutputStream;
	}

	private OutputStream irodsOutputStream;
	private final String encoding;
	/**
	 * Used in Debug mode
	 */
	private long date;

	/**
	 * Size of the socket send buffer
	 */
	public static final int OUTPUT_BUFFER_LENGTH = GeneralFileSystem
			.getWriteBufferSize();

	/**
	 * 4 bytes at the front of the header, outside XML
	 */
	public static final int HEADER_INT_LENGTH = 4;

	/**
	 * Buffer output to the socket.
	 */
	private byte outputBuffer[] = new byte[OUTPUT_BUFFER_LENGTH];

	/**
	 * Holds the offset into the outputBuffer array for adding new data.
	 */
	private int outputOffset = 0;

	static IRODSConnection instance(final IRODSAccount irodsAccount,
			String encoding) throws IRODSException, JargonException {
		IRODSConnection irodsSimpleConnection = new IRODSConnection(
				irodsAccount, encoding);
		irodsSimpleConnection.initializeConnection();
		return irodsSimpleConnection;
	}

	/**
	 * Finalizes the object by explicitly letting go of each of its internally
	 * held values.
	 *<P>
	 * 
	 * @throws IOException
	 *             If can't close socket.
	 */
	protected void finalize() throws JargonException {
		shutdown();
	}

	private void initializeConnection() throws IRODSException, JargonException {
		// connect to irods, do handshake
		// save the irods startup information to the IRODSServerProperties
		// object in the irodsConnection

		// right now this only opens the socket, the startup packet will be sent
		// by the IRODSCommands

		log.info("initializing connection with account" + irodsAccount);

		if (irodsAccount == null) {
			log.error("no irods account");
			throw new IRODSException(
					"no irods account specified, cannot connect");
		}

		log.info("irods handshake");

		connect();
		connected = true;

		// build an identifier for this connection, at least for now
		StringBuilder connectionInternalIdentifierBuilder = new StringBuilder();
		connectionInternalIdentifierBuilder.append(getConnectionUri());
		connectionInternalIdentifierBuilder.append('/');
		connectionInternalIdentifierBuilder.append(Thread.currentThread()
				.getName());
		connectionInternalIdentifierBuilder.append('/');
		connectionInternalIdentifierBuilder.append(System.currentTimeMillis());
		this.connectionInternalIdentifier = connectionInternalIdentifierBuilder
				.toString();
		log.info("connection identified as:"
				+ this.connectionInternalIdentifier);
	}

	private IRODSConnection() {
		this.irodsAccount = null;
		// this.irodsServerProperties = null;
		this.encoding = null;
	}

	private IRODSConnection(final IRODSAccount irodsAccount, String encoding) {
		this.irodsAccount = irodsAccount;
		this.encoding = encoding;
	}

	/*
	 * A simple connection to an IRODS server that is created on open, and
	 * disconnected on close. (non-Javadoc)
	 * 
	 * @see org.irods.jargon.core.connection.IRODSConnection#connect()
	 */
	private void connect() throws JargonException {
		log.info("opening socket");

		if (connected) {
			log
					.warn("doing connect when already connected!, will bypass connect and proceed");
			return;
		}

		try {
			connection = new Socket(irodsAccount.getHost(), irodsAccount
					.getPort());
			irodsInputStream = connection.getInputStream();
			irodsOutputStream = connection.getOutputStream();
		} catch (UnknownHostException e) {
			log.error("exception opening socket to:" + irodsAccount.getHost()
					+ " port:" + irodsAccount.getPort(), e);
			e.printStackTrace();
			throw new JargonException("error opening socket, unknown host", e);
		} catch (IOException e) {
			log.error("io exception opening socket to:"
					+ irodsAccount.getHost() + " port:"
					+ irodsAccount.getPort(), e);
			e.printStackTrace();
			throw new JargonException(e);
		}
		log.info("socket connected");

	}

	/*
	 * physically closing down the socket. This method will be called at the
	 * appropriate time by {@link IRODSCommands IRODSCommands} at the
	 * appropriate time.
	 * 
	 * @throws JargonException
	 */
	public void shutdown() throws JargonException {
		if (!isConnected()) {
			return;
		}
		// attempt to send shutdown packet

		try {
			connection.close();
		} catch (IOException ex) {
			log.error("IOException closing: ", ex);
		}
		connected = false;
	}

	public void obliterateConnectionAndDiscardErrors() {

		try {
			connection.shutdownInput();
		} catch (Exception e) {
			// ignore
		}

		try {
			connection.shutdownOutput();
		} catch (Exception e) {
			// ignore
		}

		try {
			connection.close();

		} catch (Exception e) {
			// ignore
		}

		connected = false;
	}

	public String getConnectionUri() throws JargonException {
		// eventually build uri from irodsAccount info
		return "irodsSimpleConnection";
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.irods.jargon.core.connection.IRODSConnection#getIRODSAccount()
	 */
	public IRODSAccount getIRODSAccount() {
		return this.irodsAccount;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.irods.jargon.core.connection.IRODSConnection#isConnected()
	 */
	public boolean isConnected() {
		return connected;
	}

	public String toString() {
		return this.connectionInternalIdentifier;
	}

	/***
	 * Jargon methods for connection *
	 **/

	/**
	 * Writes value.length bytes to this output stream.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws NullPointerException
	 *             Send buffer is empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void send(byte[] value) throws IOException {
		if (log.isDebugEnabled()) {
			log.debug("write summary: " + new String(value));
		}
		/*
		 * if (log.isTraceEnabled()) { for (int i = 0; i < value.length; i++) {
		 * log.trace("trace:" + value[i] + " "); } }
		 */

		if ((value.length + outputOffset) >= OUTPUT_BUFFER_LENGTH) {
			// in cases where OUTPUT_BUFFER_LENGTH isn't big enough
			irodsOutputStream.write(outputBuffer, 0, outputOffset);
			irodsOutputStream.write(value);
			irodsOutputStream.flush();
			outputOffset = 0;
		} else {
			// the message sent isn't longer than OUTPUT_BUFFER_LENGTH
			System
					.arraycopy(value, 0, outputBuffer, outputOffset,
							value.length);
			outputOffset += value.length;
		}
	}

	/**
	 * Writes a certain length of bytes at some offset in the value array to the
	 * output stream, by converting the value to a byte array and calling send(
	 * byte[] value ).
	 * 
	 * @param value
	 *            value to be sent
	 * @param offset
	 *            offset into array
	 * @param length
	 *            number of bytes to read
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void send(byte[] value, int offset, int length) throws IOException {
		byte temp[] = new byte[length];

		System.arraycopy(value, offset, temp, 0, length);

		send(temp);
	}

	/**
	 * Writes value.length bytes to this output stream.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void send(String value) throws IOException {
		send(value.getBytes(encoding));
	}

	/**
	 * Writes an int to the output stream as four bytes, network order (high
	 * byte first).
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void sendInNetworkOrder(int value) throws IOException {
		byte bytes[] = new byte[INT_LENGTH];

		Host.copyInt(value, bytes);
		send(bytes);
	}

	/**
	 * Writes an long to the output stream as eight bytes, low byte first.
	 * 
	 * @param value
	 *            value to be sent
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void send(InputStream source, long length) throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		while (length > 0) {
			if (temp.length > length) {
				temp = new byte[(int) length];
			}
			length -= source.read(temp, 0, temp.length);
			send(temp);
		}
	}

	/**
	 * Flushes all data in the output stream and sends it to the server.
	 * 
	 * @throws NullPointerException
	 *             Send buffer empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	void flush() throws IOException {
		if (connection.isClosed()) {
			// hopefully this isn't too slow to check.
			throw new ClosedChannelException();
		}
		irodsOutputStream.write(outputBuffer, 0, outputOffset);

		outputOffset = 0;
	}

	/**
	 * read length bytes from the server socket connection and write them to
	 * destination
	 */
	void read(OutputStream destination, long length) throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		int n = 0;
		while (length > 0) {
			n = read(temp, 0, Math.min(IRODSFileSystem.BUFFER_SIZE,
					(int) length));
			if (n > 0) {
				length -= n;
				destination.write(temp, 0, n);
			} else {
				length = n;
			}
		}
	}

	/**
	 * read length bytes from the server socket connection and write them to
	 * destination
	 */
	void read(GeneralRandomAccessFile destination, long length)
			throws IOException {
		byte[] temp = new byte[Math.min(IRODSFileSystem.BUFFER_SIZE,
				(int) length)];
		int n = 0;
		while (length > 0) {
			n = read(temp, 0, Math.min(IRODSFileSystem.BUFFER_SIZE,
					(int) length));
			if (n > 0) {
				length -= n;
				destination.write(temp, 0, n);
			} else {
				length = n;
			}
		}
	}

	/**
	 * Reads a byte array from the server. Blocks until <code>length</code>
	 * number of bytes are read.
	 * 
	 * @param length
	 *            length of byte array to be read
	 * @return byte[] bytes read from the server
	 * @throws OutOfMemoryError
	 *             Read buffer overflow
	 * @throws ClosedChannelException
	 *             if the connection is closed
	 * @throws NullPointerException
	 *             Read buffer empty
	 * @throws IOException
	 *             If an IOException occurs
	 */
	int read(byte[] value, int offset, int length)
			throws ClosedChannelException, InterruptedIOException, IOException {
		int result = 0;
		if (length + offset > value.length) {
			log
					.error("index out of bounds exception, length + offset larger then byte array");
			throw new IndexOutOfBoundsException(
					"length + offset larger than byte array");
		}
		int bytesRead = 0;
		while (bytesRead < length) {
			int read = irodsInputStream.read(value, offset + bytesRead, length
					- bytesRead);
			if (read == -1)
				break;
			bytesRead += read;
		}
		result = bytesRead;
		if (log.isDebugEnabled()) {

			if (log.isDebugEnabled()) {

				for (int i = offset; i < offset + bytesRead; i++) {
					log.debug("trace:" + value[i] + " ");
				}

			}

		}
		return result;
	}

	/**
	 * Create the iRODS header packet
	 */
	byte[] createHeader(String type, int messageLength, int errorLength,
			long byteStringLength, int intInfo) throws IOException {
		if (log.isDebugEnabled()) {
			// to print how long this function call took
			date = new Date().getTime();

			log.info("functionID: " + intInfo);
		}

		StringBuilder headerBuilder = new StringBuilder();
		headerBuilder.append("<MsgHeader_PI>");
		headerBuilder.append("<type>");
		headerBuilder.append(type);
		headerBuilder.append("</type>");
		headerBuilder.append("<msgLen>");
		headerBuilder.append(messageLength);
		headerBuilder.append("</msgLen>");
		headerBuilder.append("<errorLen>");
		headerBuilder.append(errorLength);
		headerBuilder.append("</errorLen>");
		headerBuilder.append("<bsLen>");
		headerBuilder.append(byteStringLength);
		headerBuilder.append("</bsLen>");
		headerBuilder.append("<intInfo>");
		headerBuilder.append(intInfo);
		headerBuilder.append("</intInfo>");
		headerBuilder.append("</MsgHeader_PI>");

		String header = headerBuilder.toString();

		byte[] temp = header.getBytes(encoding);
		byte[] full = new byte[4 + temp.length];
		// load first 4 byte with header length
		Host.copyInt(temp.length, full);
		// copy rest of header into full
		System.arraycopy(temp, 0, full, 4, temp.length);
		return full;
	}

	Tag readMessage() throws IOException {
		return readMessage(true);
	}

	Tag readMessage(boolean decode) throws IOException {
		log.info("reading message");
		Tag header = readHeader();
		/*
		 * if (log.isDebugEnabled()) { log.debug("header:" + header); }
		 */
		if (header == null) {
			log.error("encountered a null header alue when reading a message");
			throw new RuntimeException("header was null when reading a message");
		}

		Tag message = null;

		if (log.isInfoEnabled()) {
			// print how long this function call took
			log.info((new Date().getTime() - date) + " millisecs");
		}

		String type = header.tags[0].getStringValue();
		int messageLength = header.tags[1].getIntValue();
		int errorLength = header.tags[2].getIntValue();
		int bytesLength = header.tags[3].getIntValue();
		int info = header.tags[4].getIntValue();

		// Reports iRODS errors, throw exception if appropriate
		if (info < 0) {
			log.info("info less than zero:" + info);
			// if nothing else, read the returned bytes and throw them away
			if (messageLength > 0)
				read(new byte[messageLength], 0, messageLength);

			if (info == IRODSException.CAT_NO_ROWS_FOUND
					|| info == IRODSException.CAT_SUCCESS_BUT_WITH_NO_INFO) {
				log.info("no rows found or success with no info");
				if (errorLength != 0) {
					byte[] errorMessage = new byte[errorLength];
					read(errorMessage, 0, errorLength);
					Tag errorTag = Tag.readNextTag(errorMessage, encoding);
					log
							.warn("IRODS error occured, no rows found or success with no info "
									+ errorTag
											.getTag(IRODSConstants.RErrMsg_PI)
											.getTag(IRODSConstants.msg)
									+ " : "
									+ info);

				}

				// query with no results
				log.info("returning null from read");
				return null;
			} else if (info == IRODSException.OVERWITE_WITHOUT_FORCE_FLAG) {
				log.warn("Attempt to overwrite file without force flag. info: "
						+ info);
				throw new IRODSException(
						"Attempt to overwrite file without force flag. ", info);
			} else {
				if (errorLength != 0) {
					byte[] errorMessage = new byte[errorLength];
					read(errorMessage, 0, errorLength);
					Tag errorTag = Tag.readNextTag(errorMessage, encoding);
					log.error("IRODS error occured "
							+ errorTag.getTag(IRODSConstants.RErrMsg_PI)
									.getTag(IRODSConstants.msg) + " info:"
							+ info);

					throw new IRODSException("IRODS error occured "
							+ errorTag.getTag(IRODSConstants.RErrMsg_PI)
									.getTag(IRODSConstants.msg), info);
				}
				log.error("IRODS error occured, info:" + info);
				throw new IRODSException("IRODS error occured " + info, info);
			}
		}

		if (errorLength != 0) {
			log.warn("error length is not zero, extracting error message");
			byte[] errorMessage = new byte[errorLength];
			read(errorMessage, 0, errorLength);
			Tag errorTag = Tag.readNextTag(errorMessage, encoding);
			log.error("IRODS error occured"
					+ errorTag.getTag(IRODSConstants.RErrMsg_PI).getTag(
							IRODSConstants.msg));

			throw new IRODSException("IRODS error occured "
					+ errorTag.getTag(IRODSConstants.RErrMsg_PI).getTag(
							IRODSConstants.msg), info);
		}

		if (messageLength > 0) {
			log.info("message length gt 0 will read message body");
			message = readMessageBody(messageLength, decode);
		}

		if (bytesLength != 0 || info > 0) {
			if (message == null) {
				message = new Tag(IRODSConstants.MsgHeader_PI);
			}

			// lets the bytes get read later,
			// instead of passing a 32MB buffer around.
			message.addTag(header);
		}

		return message;
	}

	/**
	 * Going to read the header somewhat differently
	 */
	Tag readHeader() throws IOException {
		byte[] header;
		int length = readHeaderLength();
		if (length < 0)
			throw new ProtocolException();
		else if (length > 10000000) {
			/*
			 * Protocol failure: One cause, if running a rule that uses
			 * msiDataObjPut or Get, then when the server requests the Put, the
			 * client instead send a query or other function, the next function
			 * sent to the server (such as the Put itself) will fail and the
			 * server will return a RError_PI message. However this message,
			 * unlike regular server communications, does not have the 4 byte
			 * message length in front of the message. Causing unexpected
			 * results when attempting to parse the message, ie. <REr are
			 * interpreted as the message length.
			 * 
			 * <RError_PI> <count>1 </count> <RErrMsg_PI> <status>-808000
			 * </status> <msg>ERROR: msiDataObjPut: rsDataObjPut failed for
			 * <MsgHeader_PI> <type>RODS_API_REPLY </type> <msgLen>0 </msgLen>
			 * <errorLen>0 </errorLen> <bsLen>0 </bsLen> <intInfo>-808000
			 * </intInfo> </MsgHeader_PI> , status = -808000 </msg>
			 * </RErrMsg_PI> </RError_PI>
			 */

			log.error("protocol error " + length);

			// to recover from some protocol errors, (slowly and if lucky)
			// read until a new message header is found.
			boolean cont = true; // thread to eventually end this when blocked
			int searchForNewHeaderChar;
			byte[] temp = new byte[13];
			String newHeader = "MsgHeader_PI>";
			// hopefully won't be too many bytes...
			do {
				searchForNewHeaderChar = irodsInputStream.read();
				if (searchForNewHeaderChar == (int) '<') {
					searchForNewHeaderChar = irodsInputStream.read(temp);
					if (true) {// (sigh == 13) {
						if (new String(temp, encoding).equals(newHeader)) {
							temp = new byte[1000];
							// find the end of the header and proceed from there
							for (int i = 0; i < temp.length; i++) {
								temp[i] = (byte) irodsInputStream.read();
								if (temp[i] == '>' && temp[i - 1] == 'I'
										&& temp[i - 2] == 'P'
										&& temp[i - 3] == '_'
										&& temp[i - 4] == 'r'
										&& temp[i - 5] == 'e'
										&& temp[i - 6] == 'd'
										&& temp[i - 7] == 'a'
										&& temp[i - 8] == 'e'
										&& temp[i - 9] == 'H'
										&& temp[i - 10] == 'g'
										&& temp[i - 11] == 's'
										&& temp[i - 12] == 'M'
										&& temp[i - 13] == '/'
										&& temp[i - 14] == '<') {
									// account for the '\n'
									irodsInputStream.read();

									// <MsgHeader_PI> + the above header
									header = new byte[i + 1 + 14];
									System.arraycopy(("<" + newHeader)
											.getBytes(), 0, header, 0, 14);
									System
											.arraycopy(temp, 0, header, 14,
													i + 1);
									return Tag.readNextTag(header, encoding);
								}
							}
						}
					}
				} else if (searchForNewHeaderChar == -1) {
					log.error("protocol exception, server connection was lost");
					throw new ProtocolException(
							"Server connection lost, due to error");
				}
			} while (cont);

		}

		header = new byte[length];
		read(header, 0, length);

		return Tag.readNextTag(header, encoding);
	}

	int readHeaderLength() throws IOException {
		byte[] headerInt = new byte[HEADER_INT_LENGTH];
		read(headerInt, 0, HEADER_INT_LENGTH);
		return Host.castToInt(headerInt);
	}

	Tag readMessageBody(int length, boolean decode) throws IOException {
		byte[] body = new byte[length];
		read(body, 0, length);
		return Tag.readNextTag(body, decode, encoding);
	}
}
