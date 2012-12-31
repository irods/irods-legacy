/**
 * 
 */
package org.irods.jargon.core.packinstr;

/**
 * Represents options that control the transfer of data to and from iRODS (get
 * and put). The data in this object are synchronized and thread-safe.
 * <p/>
 * Note that udp options are included here, but the UDP option is not yet
 * implemented in jargon, and will have no effect. This part of the API is new
 * and subject to refactoring.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class TransferOptions {

	public enum TransferType {
		STANDARD, NO_PARALLEL, UDP
	}

	public static final int DEFAULT_UDP_SEND_RATE = 600000;
	public static final int DEFAULT_UDP_PACKET_SIZE = 8192;
	public static final int DEFAULT_MAX_PARALLEL_THREADS = 4;

	private int maxThreads = DEFAULT_MAX_PARALLEL_THREADS;
	private int udpSendRate = DEFAULT_UDP_SEND_RATE;
	private int udpPacketSize = DEFAULT_UDP_PACKET_SIZE;
	private TransferType transferType = TransferType.STANDARD;
	/**
	 * <code>boolean</code> that indicates whether the connection should be
	 * re-directed to the proper resource, based on either the given resource in
	 * a transfer operation, or based on a lookup of where data objects in a
	 * collection are physically stored. <code>false</code> by default.
	 */
	private boolean redirectToResource = false;

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("transferOptions:");
		sb.append("\n   maxThreads:");
		sb.append(maxThreads);
		sb.append("\n  transferType:");
		sb.append(transferType);
		sb.append("\n   udpSendRate:");
		sb.append(udpSendRate);
		sb.append("\n udpPacketSize:");
		sb.append(udpPacketSize);
		sb.append("\n redirectToResource:");
		sb.append(redirectToResource);
		return sb.toString();
	}

	public synchronized TransferType getTransferType() {
		return transferType;
	}

	public synchronized void setTransferType(final TransferType transferType) {
		this.transferType = transferType;
	}

	public synchronized int getMaxThreads() {
		return maxThreads;
	}

	public synchronized void setMaxThreads(final int maxThreads) {
		this.maxThreads = maxThreads;
	}

	public synchronized int getUdpSendRate() {
		return udpSendRate;
	}

	public synchronized void setUdpSendRate(final int udpSendRate) {
		this.udpSendRate = udpSendRate;
	}

	public synchronized int getUdpPacketSize() {
		return udpPacketSize;
	}

	public synchronized void setUdpPacketSize(final int udpPacketSize) {
		this.udpPacketSize = udpPacketSize;
	}

	public synchronized boolean isRedirectToResource() {
		return redirectToResource;
	}

	public synchronized void setRedirectToResource(
			final boolean redirectToResource) {
		this.redirectToResource = redirectToResource;
	}

}
