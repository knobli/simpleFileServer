package ch.zhaw.client;

import static org.junit.Assert.assertEquals;

import java.io.IOException;

import org.junit.Test;

public class UpdateFileTest extends Testbase {

	@Test
	public void updateFileWithSpaceInContent() throws IOException {
		getClient().createFile("testFile", "Test content");
		ReturnValue returnValue = getClient().updateFile("testFile", "new content");
		assertEquals(Protocol.ANSWER_SUCCESS_UPDATE,returnValue.getRetrunValue());		
	}
	
	@Test
	public void updateFileWithoutSpaceInContent() throws IOException {
		getClient().createFile("testFile2", "Test_content");
		ReturnValue returnValue = getClient().updateFile("testFile2", "new_content");
		assertEquals(Protocol.ANSWER_SUCCESS_UPDATE,returnValue.getRetrunValue());
	}	

	@Test
	public void updateFileThatNotExists() throws IOException {
		ReturnValue returnValue = getClient().updateFile("noFile", "dummy");
		assertEquals(Protocol.ANSWER_FAILED_UPDATE,returnValue.getRetrunValue());
	}			
	
}
