package ch.zhaw.client;

import java.util.List;

public class FileListReturnValue extends ReturnValue {
	
	private Integer fileCount;
	private List<String> fileNames;
	
	
	public FileListReturnValue(Integer fileCount, List<String> fileNames) {
		super();
		setAction(ActionType.LIST);
		this.setFileCount(fileCount);
		this.setFileNames(fileNames);
	}


	public FileListReturnValue() {
		this(null, null);
	}


	public Integer getFileCount() {
		return fileCount;
	}


	public void setFileCount(Integer fileCount) {
		this.fileCount = fileCount;
	}


	public List<String> getFileNames() {
		return fileNames;
	}


	public void setFileNames(List<String> fileNames) {
		this.fileNames = fileNames;
	}
	
	

}
