package ch.zhaw.client;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ch.zhaw.client.utilities.ServerConnector;

public class Client {

	private static final Logger LOGGER = Logger.getLogger(Client.class
			.toString());
	
	private ServerConnector serverConnector;
	
	static {
		LOGGER.setLevel(Level.FINEST);
		ConsoleHandler handler = new ConsoleHandler();
		handler.setFormatter(new SimpleFormatter());
		handler.setLevel(Level.ALL);
		LOGGER.addHandler(handler);
	}
	
	public Client() {
		serverConnector = new ServerConnector();
	}

	public ReturnValue createFile(String filename, String content)
			throws IOException {
		String message = createCreateFileMessage(filename, content);
		List<String> rv = sendToServer(message);
		return new ReturnValue(rv.get(0),ActionType.CREATE);
	}

	public String createCreateFileMessage(String filename, String content) {
		String message = Protocol.COMMAND_CREATE;
		message = message.replaceAll(Protocol.PLACEHOLDER_FILENAME, filename);
		message = message.replaceAll(Protocol.PLACEHOLDER_LENGTH, "" + (content.length() + 1));
		message = message.replaceAll(Protocol.PLACEHOLDER_CONTENT, content);
		return message;
	}

	public FileReturnValue readFile(String filename) throws IOException {
		String message = createReadFileMessage(filename);
		List<String> rv = sendToServer(message);
		FileReturnValue returnValue = new FileReturnValue(rv.get(0),"");
		if(rv.size() == 2){
			returnValue.setContent(rv.get(1));
		}
		return returnValue;
	}

	public String createReadFileMessage(String filename) {
		String message = Protocol.COMMAND_READ;
		message = message.replaceAll(Protocol.PLACEHOLDER_FILENAME, filename);
		return message;
	}

	public ReturnValue updateFile(String filename, String content)
			throws IOException {
		String message = createUpdateFileMessage(filename, content);
		List<String> rv = sendToServer(message);
		return new ReturnValue(rv.get(0),ActionType.UPDATE);		
	}

	public String createUpdateFileMessage(String filename, String content) {
		String message = Protocol.COMMAND_UPDATE;
		message = message.replaceAll(Protocol.PLACEHOLDER_FILENAME, filename);
		message = message.replaceAll(Protocol.PLACEHOLDER_LENGTH, "" + (content.length() + 1));
		message = message.replaceAll(Protocol.PLACEHOLDER_CONTENT, content);
		return message;
	}
	
	public ReturnValue deleteFile(String filename) throws IOException{
		String message = createDeleteFileMessage(filename);
		List<String> rv = sendToServer(message);
		return new ReturnValue(rv.get(0),ActionType.DELETE);	
	}
	
	public String createDeleteFileMessage(String filename) {
		String message = Protocol.COMMAND_DELETE;
		message = message.replaceAll(Protocol.PLACEHOLDER_FILENAME, filename);
		return message;
	}

	public FileListReturnValue listFiles() throws IOException {
		String message = Protocol.COMMAND_LIST;
		List<String> returnValue = sendToServer(message);
		FileListReturnValue fileList = new FileListReturnValue();
		fileList.setRetrunValue(returnValue.get(0));
		Pattern p = Pattern.compile(Protocol.REGEX_LIST_ANSWER);
		Matcher m = p.matcher(returnValue.get(0));
		m.find();
		String fileCount = m.group(1);
		fileList.setFileCount(Integer.parseInt(fileCount));
		returnValue.remove(0);
		fileList.setFileNames(returnValue);
		return fileList;

	}

	public List<String> sendToServer(String message) throws IOException {
		List<String> returnValue = new ArrayList<String>();
		try {
			if (getServerConnector().connect()) {
				returnValue = getServerConnector().sendMessage(message);
			}
		} finally {
			getServerConnector().disconnect();
		}
		if(returnValue.size() == 0){
			throw new RuntimeException("Invalid response for message: " + message);
		} else if ( returnValue.get(0).contains(Protocol.ANSWER_UNKOWN)){
			throw new RuntimeException("Unkown command: " + message + " return value: " + returnValue.get(0));
		}
		return returnValue;
	}
	
	public ServerConnector getServerConnector(){
		return this.serverConnector;
	}
	
}
