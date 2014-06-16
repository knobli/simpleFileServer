package ch.zhaw.client;

import static org.junit.Assert.assertEquals;

import java.io.IOException;

import org.junit.Test;

public class DeleteFileTest extends Testbase {

	@Test
	public void deleteFileWithSpaceInContent() throws IOException {
		getClient().createFile("testFile", "Test content");
		ReturnValue returnValue = getClient().deleteFile("testFile");
		assertEquals(Protocol.ANSWER_SUCCESS_DELETE,returnValue.getRetrunValue());
	}
	
	@Test
	public void deleteFileWithoutSpaceInContent() throws IOException {
		getClient().createFile("testFile2", "Test_content");
		ReturnValue returnValue = getClient().deleteFile("testFile2");
		assertEquals(Protocol.ANSWER_SUCCESS_DELETE,returnValue.getRetrunValue());
	}
	
	@Test
	public void deleteFileThatNotExists() throws IOException {
		ReturnValue returnValue = getClient().deleteFile("noFile");
		assertEquals(Protocol.ANSWER_FAILED_DELETE,returnValue.getRetrunValue());
	}		

}
