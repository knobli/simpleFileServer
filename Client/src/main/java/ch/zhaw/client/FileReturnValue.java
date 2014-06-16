package ch.zhaw.client;

public class FileReturnValue extends ReturnValue {
	
	private String content;
	
	public FileReturnValue(){
		this(null,null);
	}
	
	public FileReturnValue(String retrunValue, String content) {
		super(retrunValue, ActionType.READ);
		this.setContent(content);
	}

	public String getContent() {
		return content;
	}

	public void setContent(String content) {
		this.content = content;
	}

}
