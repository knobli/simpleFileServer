package ch.zhaw.client;

public class Protocol {
	public static final String ANSWER_SUCCESS_LIST = "ACK";
	@ProtocolAnnotation(action=ActionType.CREATE)
	public static final String ANSWER_FAILED_CREATE = "FILEEXISTS";
	@ProtocolAnnotation(action=ActionType.CREATE)
	public static final String ANSWER_SUCCESS_CREATE = "FILECREATED";
	@ProtocolAnnotation(action=ActionType.READ)
	public static final String ANSWER_FAILED_READ = "NOSUCHFILE";
	
	public static final String ANSWER_SUCCESS_READ = "FILECONTENT __FILENAME__ __LENGTH__";
	@ProtocolAnnotation(action=ActionType.UPDATE)
	public static final String ANSWER_FAILED_UPDATE = "NOSUCHFILE";
	@ProtocolAnnotation(action=ActionType.UPDATE)
	public static final String ANSWER_SUCCESS_UPDATE = "UPDATED";
	@ProtocolAnnotation(action=ActionType.DELETE)
	public static final String ANSWER_FAILED_DELETE = "NOSUCHFILE";
	@ProtocolAnnotation(action=ActionType.DELETE)
	public static final String ANSWER_SUCCESS_DELETE = "DELETED";
	
	public static final String ANSWER_UNKOWN = "UNKOWN";
	
	@ProtocolAnnotation(action=ActionType.LIST)
	public static final String ANSWER_REGEX_SUCCESS_LIST = ANSWER_SUCCESS_LIST + "\\s+[\\d]+";

	@ProtocolAnnotation(action=ActionType.READ)
	public static final String ANSWER_REGEX_SUCCESS_READ = "FILECONTENT\\s+[\\w\\d\\t\\x0b\\r\\f]+\\s+[\\d]+";

	public static final String PLACEHOLDER_FILENAME = "__FILENAME__";
	public static final String PLACEHOLDER_LENGTH = "__LENGTH__";
	public static final String PLACEHOLDER_CONTENT = "__CONTENT__";

	public static final String COMMAND_LIST = "LIST\n";
	public static final String COMMAND_CREATE = "CREATE "
			+ PLACEHOLDER_FILENAME + " " + PLACEHOLDER_LENGTH + "\n"
			+ PLACEHOLDER_CONTENT + "\n";
	public static final String COMMAND_READ = "READ " + PLACEHOLDER_FILENAME
			+ "\n";
	public static final String COMMAND_UPDATE = "UPDATE "
			+ PLACEHOLDER_FILENAME + " " + PLACEHOLDER_LENGTH + "\n"
			+ PLACEHOLDER_CONTENT + "\n";
	public static final String COMMAND_DELETE = "DELETE "
			+ PLACEHOLDER_FILENAME + "\n";

	public static final String REGEX_LIST_ANSWER = ANSWER_SUCCESS_LIST
			+ " (\\d+)";

}