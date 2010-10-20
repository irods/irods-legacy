package org.irods.jargon.core.packinstr;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class CollInpTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testInstance() throws Exception {
		CollInp collInp = CollInp.instance("testcollname", true);
		Assert.assertNotNull("null coll inp object", collInp);
	}

	@Test(expected = JargonException.class)
	public final void testInstanceNullPaty() throws Exception {
		@SuppressWarnings("unused")
		CollInp collInp = CollInp.instance(null, true);
	}

	@Test
	public final void testGetParsedTags() throws Exception {
		StringBuilder sb = new StringBuilder();
		sb.append("<CollInpNew_PI><collName>testcollname</collName>\n");
		sb.append("<flags>0</flags>\n");
		sb.append("<oprType>0</oprType>\n");
		sb.append("<KeyValPair_PI><ssLen>1</ssLen>\n");
		sb.append("<keyWord>recursiveOpr</keyWord>\n");
		sb.append("<svalue></svalue>\n");
		sb.append("</KeyValPair_PI>\n");
		sb.append("</CollInpNew_PI>\n");
		String expected = sb.toString();
		CollInp collInp = CollInp.instance("testcollname", true);
		Assert.assertEquals("did not get expected xml", expected, collInp.getParsedTags());
	}

	@Test
	public final void testGetCollectionName() throws JargonException {
		CollInp collInp = CollInp.instance("testcollname", true);
		Assert.assertEquals("coll name not set", "testcollname", collInp
				.getCollectionName());
	}

}
