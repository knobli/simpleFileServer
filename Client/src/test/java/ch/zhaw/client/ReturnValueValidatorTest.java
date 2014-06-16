package ch.zhaw.client;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

public class ReturnValueValidatorTest {

	@Test
	public void readReturnValueWithCamelCaseFileName() {
		ReturnValue rv = new ReturnValue("FILECONTENT blablaFIle", ActionType.READ);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void readReturnNoSuchFile() {
		ReturnValue rv = new ReturnValue("NOSUCHFILE", ActionType.READ);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}	
	
	@Test
	public void readReturnValueWithWrongBeginning() {	
		ReturnValue rv = new ReturnValue("FILECONTEN blablaF%s", ActionType.READ);
		assertFalse(ReturnValueValidator.validateReturnValue(rv));
	}	
	
	@Test
	public void createReturnFileExists() {
		ReturnValue rv = new ReturnValue("FILEEXISTS", ActionType.CREATE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void createReturnFileCreated() {
		ReturnValue rv = new ReturnValue("FILECREATED", ActionType.CREATE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void createReturnWrongValue() {	
		ReturnValue rv = new ReturnValue("FILECONTENT", ActionType.CREATE);
		assertFalse(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void updateReturnUpdated() {	
		ReturnValue rv = new ReturnValue("UPDATED", ActionType.UPDATE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}

	@Test
	public void updateReturnNoSuchFile() {	
		ReturnValue rv = new ReturnValue("NOSUCHFILE", ActionType.UPDATE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void updateReturnWrongValue() {	
		ReturnValue rv = new ReturnValue("FILECONTENT", ActionType.UPDATE);
		assertFalse(ReturnValueValidator.validateReturnValue(rv));
	}
	
	public void deleteReturnDeleted() {	
		ReturnValue rv = new ReturnValue("DELETED", ActionType.DELETE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}

	@Test
	public void deleteReturnNoSuchFile() {	
		ReturnValue rv = new ReturnValue("NOSUCHFILE", ActionType.DELETE);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void deleteReturnWrongValue() {	
		ReturnValue rv = new ReturnValue("FILECONTENT", ActionType.DELETE);
		assertFalse(ReturnValueValidator.validateReturnValue(rv));
	}	
	
	@Test
	public void listReturnNoSuchFile() {	
		ReturnValue rv = new ReturnValue("ACK 3", ActionType.LIST);
		assertTrue(ReturnValueValidator.validateReturnValue(rv));
	}
	
	@Test
	public void listReturnWrongValue() {	
		ReturnValue rv = new ReturnValue("ACK", ActionType.LIST);
		assertFalse(ReturnValueValidator.validateReturnValue(rv));
	}	
}
