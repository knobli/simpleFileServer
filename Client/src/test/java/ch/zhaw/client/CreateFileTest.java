package ch.zhaw.client;

import static org.junit.Assert.assertEquals;

import java.io.IOException;

import org.junit.Test;

public class CreateFileTest extends Testbase {

	@Test
	public void createFileWithSpaceInContent() throws IOException {
		ReturnValue returnValue = getClient().createFile("testFile", "Test content");
		assertEquals(Protocol.ANSWER_SUCCESS_CREATE,returnValue.getRetrunValue());
	}
	
	@Test
	public void createFileWithoutSpaceInContent() throws IOException {
		ReturnValue returnValue = getClient().createFile("testFile2", "Test_content");
		assertEquals(Protocol.ANSWER_SUCCESS_CREATE,returnValue.getRetrunValue());
	}	

}
