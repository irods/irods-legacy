//	Copyright (c) 2005, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//	JargonTreeModel.java	-  edu.sdsc.grid.gui.JargonTreeModel
//
//  CLASS HIERARCHY
//	java.lang.Object (+ javax.swing.tree.TreeModel)
//	    |
//	    +-edu.sdsc.grid.gui.JargonTreeModel
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.grid.io.local.LocalMetaDataRecordList; //TODO metadata check and generalization
import edu.sdsc.grid.io.srb.SRBFile;
import edu.sdsc.grid.io.srb.SRBMetaDataSet;

import javax.swing.JComponent;

import java.io.IOException;

import java.lang.reflect.Array;

import java.util.EventListener;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Vector;

import javax.swing.Icon;
import javax.swing.JTable;
import javax.swing.plaf.metal.MetalIconFactory;

import javax.swing.event.EventListenerList;
import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

/**
 * The class extends the default tree model to use GeneralFiles as the nodes of
 * the tree. Optimization code was added to reduce the number of remote system
 * calls made during the building, expanding or redrawing of the tree. <BR>
 * <BR>
 * This class uses a non-thread-safe storage, to keep the javax.swing classes
 * from constantly querying the filesystem for every refresh and change. All
 * those network calls really hurt performance. Unfortunately, as this class is
 * no longer thread safe, changes to the filesystem won't be displayed until the
 * model is updated.
 * 
 * @author Lucas Ammon Gilbert
 * @see JargonGui
 * @since version 1.4
 * @deprecated - GUI code will go away in future releases
 */
@Deprecated
public class JargonTreeModel implements TreeModel {
	// ----------------------------------------------------------------------
	// Constants
	// ----------------------------------------------------------------------
	/**
	 * Top level name for trees with multiple GeneralFile roots.
	 */
	public static String TOP_LEVEL = "DataGrid";

	// ----------------------------------------------------------------------
	// Fields
	// ----------------------------------------------------------------------
	/**
	 * Root of the tree.
	 */
	private Object root;

	/**
	 * Stores 1 level down for trees with multiple GeneralFile roots.
	 */
	private Object[] subRoots;

	/**
	 * The listeners
	 */
	protected EventListenerList listenerList = new EventListenerList();

	/**
	 * If metadata was selected to be returned witht he tree, i.e. if a
	 * MetaDataSelect[] was passed to the constructor.
	 */
	private boolean showMetaData = false;

	/**
	 * The metadata values to be returned by the query. FILE_NAME and
	 * DIRECTORY_NAME may be added to this list. The list may also be reorder.
	 * So fields[] is also stored to track the shown metadata and their order as
	 * returned, which may be different then the order sent in the query.
	 */
	private MetaDataCondition[] conditions;

	/**
	 * The metadata values to be returned by the query. FILE_NAME and
	 * DIRECTORY_NAME may be added to this list. The list may also be reorder.
	 * So fields[] is also stored to track the shown metadata and their order as
	 * returned, which may be different then the order sent in the query.
	 */
	private MetaDataSelect[] selects;

	/**
	 * Stored to track the displayed metadata and the order as returned, which
	 * may be different then the order sent in the query.
	 */
	private MetaDataField fields[];

	/**
	 * Arrary length of <code>fields</code>.
	 */
	private int fieldsLength;

	/*
	 * These hash objects are the complicated little storage situation, to keep
	 * the javax.swing stuff from constantly querying the filesystem for every
	 * refresh and change.
	 */
	private Hashtable table = new Hashtable();

	// TODO think about putting these in one thing...somehow
	private HashSet nodeSet = new HashSet();
	private HashSet leafSet = new HashSet();

	private Hashtable resultTable = new Hashtable();

	/**
	 * The icon displayed for folder nodes, non-leaf nodes.
	 */
	public static Icon FOLDER_ICON = new MetalIconFactory.FolderIcon16();

	/**
	 * The icon displayed for file nodes, leaf nodes.
	 */
	public static Icon FILE_ICON = new MetalIconFactory.FileIcon16();

	// ----------------------------------------------------------------------
	// Constructors
	// ----------------------------------------------------------------------
	/**
	 * Starts the tree with this GeneralFile as the root node.
	 */
	public JargonTreeModel(GeneralFile root) {
		super();
		this.root = root;

		selects = new MetaDataSelect[2];
		selects[0] = MetaDataSet.newSelection(GeneralMetaData.FILE_NAME);
		selects[1] = MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME);
	}

	/**
	 * Starts the tree with a fake root node, <code>TOP_LEVEL</code>, with the
	 * <code>roots</code> as children to that root.
	 */
	public JargonTreeModel(GeneralFile[] roots) {
		super();
		root = TOP_LEVEL;
		subRoots = roots;

		selects = new MetaDataSelect[2];
		selects[0] = MetaDataSet.newSelection(GeneralMetaData.FILE_NAME);
		selects[1] = MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME);
	}

	/**
	 * Starts the tree with this GeneralFile as the root node. Only file nodes
	 * which fit the conditions will be displayed on the tree.
	 */
	public JargonTreeModel(GeneralFile root, MetaDataCondition[] conditions)
			throws IOException {
		super();
		this.root = root;

		selects = new MetaDataSelect[2];
		selects[0] = MetaDataSet.newSelection(GeneralMetaData.FILE_NAME);
		selects[1] = MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME);

		this.conditions = conditions;
	}

	/**
	 * Starts the tree with a fake root node, <code>TOP_LEVEL</code>, with the
	 * <code>roots</code> as children to that root. Only file nodes which fit
	 * the conditions will be displayed on the tree.
	 */
	public JargonTreeModel(GeneralFile[] roots, MetaDataCondition[] conditions)
			throws IOException {
		super();
		root = TOP_LEVEL;
		subRoots = roots;

		selects = new MetaDataSelect[2];
		selects[0] = MetaDataSet.newSelection(GeneralMetaData.FILE_NAME);
		selects[1] = MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME);

		this.conditions = conditions;
	}

	/**
	 * Starts the tree with this GeneralFile as the root node. File nodes on the
	 * tree will list the metadata described by <code>selects</code>
	 */
	public JargonTreeModel(GeneralFile root, MetaDataSelect[] selects)
			throws IOException {
		super();
		this.root = root;

		if (selects == null) {
			this.selects = new MetaDataSelect[2];
			this.selects[0] = MetaDataSet
					.newSelection(GeneralMetaData.FILE_NAME);
			this.selects[1] = MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME);
		} else {
			showMetaData = loadFields(root, null, selects);
		}
	}

	/**
	 * Starts the tree with a fake root node, <code>TOP_LEVEL</code>, with the
	 * <code>roots</code> as children to that root. File nodes on the tree will
	 * list the metadata described by <code>selects</code>
	 */
	public JargonTreeModel(GeneralFile[] roots, MetaDataSelect[] selects)
			throws IOException {
		super();
		root = TOP_LEVEL;
		subRoots = roots;

		if (selects == null) {
			this.selects = new MetaDataSelect[2];
			this.selects[0] = MetaDataSet
					.newSelection(GeneralMetaData.FILE_NAME);
			this.selects[1] = MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME);
		} else {
			for (int i = 0; i < roots.length; i++) {
				showMetaData = showMetaData
						|| loadFields(roots[i], null, selects);
			}
		}
	}

	/**
	 * Starts the tree with this GeneralFile as the root node. Only file nodes
	 * which fit the conditions will be displayed on the tree. File nodes on the
	 * tree will list the metadata described by <code>selects</code>
	 */
	public JargonTreeModel(GeneralFile root, MetaDataCondition[] conditions,
			MetaDataSelect[] selects) throws IOException {
		super();
		this.root = root;

		this.conditions = conditions;

		if (selects == null) {
			this.selects = new MetaDataSelect[2];
			this.selects[0] = MetaDataSet
					.newSelection(GeneralMetaData.FILE_NAME);
			this.selects[1] = MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME);
		} else {
			showMetaData = loadFields(root, conditions, selects);
		}
	}

	/**
	 * Starts the tree with a fake root node, <code>TOP_LEVEL</code>, with the
	 * <code>roots</code> as children to that root. Only file nodes which fit
	 * the conditions will be displayed on the tree. File nodes on the tree will
	 * list the metadata described by <code>selects</code>
	 */
	public JargonTreeModel(GeneralFile[] roots, MetaDataCondition[] conditions,
			MetaDataSelect[] selects) throws IOException {
		super();
		root = TOP_LEVEL;
		subRoots = roots;

		this.conditions = conditions;

		if (selects == null) {
			this.selects = new MetaDataSelect[2];
			this.selects[0] = MetaDataSet
					.newSelection(GeneralMetaData.FILE_NAME);
			this.selects[1] = MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME);
		} else {
			for (int i = 0; i < roots.length; i++) {
				showMetaData = showMetaData
						|| loadFields(roots[i], conditions, selects);
			}
		}
	}

	// ----------------------------------------------------------------------
	// Listener methods
	// ----------------------------------------------------------------------
	/**
	 * Adds a listener for the TreeModelEvent posted after the tree changes.
	 */
	public void addTreeModelListener(TreeModelListener listener) {
		listenerList.add(TreeModelListener.class, listener);
	}

	/**
	 * The only event raised by this model is TreeStructureChanged with the root
	 * as path, i.e. the whole tree has changed.
	 */
	protected void fireTreeStructureChanged(Object oldRoot) {
		TreeModelEvent event = new TreeModelEvent(this,
				new Object[] { oldRoot });
		EventListener[] listeners = listenerList
				.getListeners(TreeModelListener.class);

		for (int i = 0; i < listeners.length; i++) {
			((TreeModelListener) listeners[i]).treeStructureChanged(event);
		}

	}

	/**
	 * Removes a listener previously added with addTreeModelListener().
	 */
	public void removeTreeModelListener(TreeModelListener l) {
		listenerList.remove(TreeModelListener.class, l);
	}

	// ----------------------------------------------------------------------
	// TreeModel interface implementation
	// ----------------------------------------------------------------------
	/**
	 * Change the root of the tree.
	 * 
	 * @throws ClassCastException
	 *             if newRoot is not a GeneralFile
	 * @throws NullPointerException
	 *             if newRoot is null
	 */
	public void setRoot(Object newRoot) {
		Object oldRoot = root;

		if (newRoot != null) {
			if (newRoot instanceof GeneralFile) {
				root = newRoot;

				fireTreeStructureChanged(oldRoot);
			} else {
				throw new ClassCastException(
						"Root must be an instance of GeneralFile");
			}
		} else {
			throw new NullPointerException("Root cannot be null");
		}
	}

	/**
	 * Returns the root of the tree. Returns null only if the tree has no nodes.
	 */
	public Object getRoot() {
		return root;
	}

	/**
	 * Returns the child of parent at index in the parent's child array. Parent
	 * must be a node previously obtained from this data source. This should not
	 * return null if index is a valid index for parent (that is index >= 0 &&
	 * index < getChildCount(parent)).
	 */
	public Object getChild(Object parent, int index) {
		// if (index==0)
		// {
		// MetaDataDisplay header = new MetaDataDisplay(
		// (MetaDataRecordList)resultTable.get( (GeneralFile)parent ) );
		// return header.getTableHeader();
		// }

		/*
		 * 
		 * if (index==0) { JComponent currentComponent = new
		 * MetaDataDisplay(file, rl[0]); return
		 * ((JTable)currentComponent).getTableHeader(null); }
		 */

		System.out.println("index in getChild is " + index);
		if (parent instanceof String) {
			if (parent.equals(TOP_LEVEL)) {
				System.out.println("1st if statement = " + subRoots[index]);
				return subRoots[index];
			}
		} else if (parent instanceof LocalFile) {
			String[] children = ((LocalFile) parent).list();
			if ((children == null) || (index >= children.length))
				return null;
			// System.out.println("2nd if statement = " + LocalFile((LocalFile)
			// parent, children[index]) );
			return "asdf";// new LocalFile((LocalFile) parent, children[index]);
		}
		// else if (index==0 && (parent instanceof GeneralFile) ) {
		// System.out.println("going here");
		// MetaDataDisplay header = new MetaDataDisplay(
		// (MetaDataRecordList)resultTable.get( (GeneralFile)parent ) );
		// System.out.println("3rd if statement = " + header.getTableHeader() );
		// return header.getTableHeader();
		// }

		System.out.println("testing ");

		String[] list = (String[]) table.get(parent);

		if (list == null) {
			list = ((GeneralFile) parent).list(conditions);
			table.put(parent, list);
		}
		System.out.println("reached end of method = "
				+ FileFactory.newFile(((GeneralFile) parent), list[index]));
		return FileFactory.newFile(((GeneralFile) parent), list[index]);
	}

	/**
	 * Returns the number of children of parent. Returns 0 if the node is a leaf
	 * or if it has no children. Parent must be a node previously obtained from
	 * this data source.
	 */
	public int getChildCount(Object parent) {
		if (parent instanceof String) {
			if (parent.equals(TOP_LEVEL)) {
				return subRoots.length;
			}
		} else if (parent instanceof LocalFile) {
			String[] children = ((LocalFile) parent).list();
			if (children == null)
				return 0;
			return children.length;
		}

		String[] list = (String[]) table.get(parent);

		if (list == null) {
			store(parent);

			list = ((GeneralFile) parent).list(conditions);
			if (list != null)
				table.put(parent, list);
		}

		if (list != null) {
			System.out.println("list is " + list.length);
			return list.length; // /// if add +1 here, messes up table
		} else
			return 0;
	}

	/**
	 * Returns the index of child in parent.
	 */
	public int getIndexOfChild(Object parent, Object child) {
		if (parent instanceof String) {
			if (parent.equals(TOP_LEVEL)) {
				// TODO inefficient?
				for (int i = 0; i < subRoots.length; i++) {
					if (child.equals(subRoots[i])) {
						return i;
					}
				}
				for (int i = 0; i < subRoots.length; i++) {
					if (subRoots[i].getClass().equals(child.getClass())) {
						parent = subRoots[i];
					}
				}

			}
		} else if (parent instanceof LocalFile) {
			String[] children = ((LocalFile) parent).list();
			if (children == null)
				return -1;
			String childname = ((LocalFile) child).getName();
			for (int i = 0; i < children.length; i++) {
				if (childname.equals(children[i]))
					return i;
			}
			return -1;
		}

		String kid = ((GeneralFile) child).getName();
		String[] list = (String[]) table.get(parent);

		if (list == null) {
			list = ((GeneralFile) parent).list(conditions);
			table.put(parent, list);
		}

		for (int i = 0; i < list.length; i++) {
			if (list[i].equals(kid)) {
				System.out.println(" i is " + i);
				return i;
			}
		}
		return -1;
	}

	/**
	 * Returns true if node is a leaf. It is possible for this method to return
	 * false even if node has no children. A directory in a filesystem, for
	 * example, may contain no files; the node representing the directory is not
	 * a leaf, but it also has no children.
	 */
	public boolean isLeaf(Object node) { // //////// calls getChild
		if (node instanceof String) {
			if (node.equals(TOP_LEVEL)) {
				return false;
			}
		} else if (node instanceof LocalFile) {
			return ((LocalFile) node).isFile();
		}

		if (leafSet.contains(node))
			return true;
		if (nodeSet.contains(node))
			return false;

		if (node instanceof GeneralFile) {
			GeneralFile file = (GeneralFile) node;

			if (file.isDirectory()) {
				nodeSet.add(node);

				return false;
			} else {
				leafSet.add(node);
				return true;
			}
		} else
			return true;
	}

	/**
	 * Messaged when the user has altered the value for the item identified by
	 * path to newValue. If newValue signifies a truly new value the model
	 * should post a treeNodesChanged event.
	 */
	public void valueForPathChanged(TreePath path, Object newValue) {
		// TODO post event
		System.out
				.println("valueForPathChanged : " + path + " --> " + newValue);
	}

	/**
	 * Load all the crazy metadata hashtables and hashsets for objects which can
	 * handle metadata.
	 */
	private void store(Object obj) {
		// TODO metadata usable check
		if (obj instanceof SRBFile) {
			GeneralFileSystem fileSystem = ((GeneralFile) obj).getFileSystem();
			GeneralFile tempFile = null;
			String path = ((GeneralFile) obj).getAbsolutePath();
			MetaDataRecordList[] rl1 = null;
			MetaDataRecordList[] rl2 = null;
			MetaDataCondition tempConditions[] = { MetaDataSet.newCondition(
					GeneralMetaData.DIRECTORY_NAME, MetaDataCondition.EQUAL,
					path) };
			MetaDataSelect dirOnlySelects[] = { MetaDataSet
					.newSelection(GeneralMetaData.DIRECTORY_NAME) };

			try {
				// Have to do two queries, one for files and one for
				// directories.
				// get all the files
				tempConditions = MetaDataSet.mergeConditions(conditions,
						tempConditions);

				rl1 = (MetaDataRecordList[]) fileSystem.query(tempConditions,
						selects);
				rl1 = MetaDataRecordList.getAllResults(rl1);

				if (rl1 != null) {
					for (int i = 0; i < rl1.length; i++) {
						tempFile = FileFactory.newFile(fileSystem, rl1[i]
								.getValue(GeneralMetaData.DIRECTORY_NAME)
								.toString(), rl1[i].getValue(
								GeneralMetaData.FILE_NAME).toString());

						// add all the directories to the leaf hash
						leafSet.add(tempFile);

						if (showMetaData) {
							// save the metadata to the resultTable hash
							resultTable.put(tempFile, rl1[i]);
						}
					}
				}

				// get all the sub-directories
				tempConditions = new MetaDataCondition[1];
				tempConditions[0] = MetaDataSet.newCondition(
						SRBMetaDataSet.PARENT_DIRECTORY_NAME,
						MetaDataCondition.EQUAL, path);

				rl2 = (MetaDataRecordList[]) fileSystem.query(tempConditions,
						dirOnlySelects);
				rl2 = MetaDataRecordList.getAllResults(rl2);

				if (rl2 != null) {
					for (int i = 0; i < rl2.length; i++) {
						tempFile = FileFactory.newFile(fileSystem, rl2[i]
								.getValue(GeneralMetaData.DIRECTORY_NAME)
								.toString());
						// only one record per rl
						nodeSet.add(tempFile);
						if (showMetaData) {
							// save the metadata to the resultTable hash
							resultTable.put(tempFile, rl2[i]);
						}
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	// ----------------------------------------------------------------------
	// More MetaData Methods
	// ----------------------------------------------------------------------
	/**
	 * Gets order of fields for future use.
	 */
	private boolean loadFields(GeneralFile root,
			MetaDataCondition[] conditions, MetaDataSelect[] selects)
			throws IOException {
		// TODO? GeneralFile.queriable
		if (root instanceof LocalFile)
			return false;

		GeneralFileSystem fileSystem = root.getFileSystem();
		MetaDataRecordList[] rl = null;
		MetaDataSelect[] tempSelects = {
				MetaDataSet.newSelection(GeneralMetaData.FILE_NAME),
				MetaDataSet.newSelection(GeneralMetaData.DIRECTORY_NAME), };
		selects = MetaDataSet.mergeSelects(selects, tempSelects);

		// remove duplicate/null selects.
		selects = (MetaDataSelect[]) cleanNulls(selects);
		if (selects == null)
			throw new NullPointerException("No valid Selects");

		// make final list of selects.
		this.selects = selects;

		if (conditions == null) {
			// TODO TODO! not a very efficent way...
			conditions = new MetaDataCondition[1];
			// get all the sub-directories
			conditions[0] = MetaDataSet.newCondition(GeneralMetaData.SIZE,
					MetaDataCondition.EQUAL, "0");

			rl = fileSystem.query(conditions, selects, 1);

			if (rl == null) {
				// the SIZE NOT_EQUAL 0 query is even slower, but always returns
				// something.
				conditions[0] = MetaDataSet.newCondition(GeneralMetaData.SIZE,
						MetaDataCondition.NOT_EQUAL, "0");
				rl = fileSystem.query(conditions, selects, 1);
			}
		} else {
			rl = fileSystem.query(conditions, selects, 1);
		}

		if (rl != null) {
			int length = rl[0].getFieldCount();

			fields = new MetaDataField[length];
			for (int i = 0; i < length; i++) {
				fields[i] = rl[0].getField(i);
			}
			fieldsLength = fields.length;
			return true;
		} else {
			// just isn't working, maybe GeneralFile subclass not have metadata.
			return false;
		}
	}

	/**
	 * Returns the metadata results for this file as a JTable. Returns null if
	 * the file has not already been displayed by the treemodel.
	 * 
	 * @see JargonTreeCellRenderer.getTreeCellRendererComponent
	 */
	JTable getMetaData(GeneralFile node) {
		// TODO not really sure I want to make public.
		// this method can still be recreated without making it public.
		GeneralFile file = (GeneralFile) node;
		MetaDataRecordList rl = (MetaDataRecordList) resultTable.get(file);
		if (rl == null)
			return null;

		int fieldCount = rl.getFieldCount();
		Object[][] tableData = new Object[1][fieldCount];
		String[] tableNames = new String[fieldCount];

		for (int i = 0; i < fieldCount; i++) {
			tableNames[i] = rl.getFieldName(i);

			tableData[0][i] = rl.getValue(i);
		}

		return new JTable(tableData, tableNames);
	}

	/**
	 * Returns the metadata query result associated with this node of the tree.
	 * Or null if <code>node</code> has not been queried or does not exist.
	 * 
	 * @see JargonTreeCellEditor#getTreeCellEditorComponent
	 */
	public MetaDataRecordList getMetaDataRecordList(Object node) {
		return (MetaDataRecordList) resultTable.get(node);
	}

	/**
	 * Returns if this treemodel will show metadata.
	 */
	public boolean showMetaData() {
		return showMetaData;
	}

	// TODO taken from SRBMetaDataCommands, could probably be static and public.
	// TODO ...and this must exist somewhere else in some standard java method.
	/**
	 * Takes an array and removes null and duplicate values
	 */
	static final Object[] cleanNulls(Object[] obj) {
		Vector temp = new Vector(obj.length);
		boolean add = true;
		int i = 0, j = 0;

		for (i = 0; i < obj.length; i++) {
			if (obj[i] != null) {
				for (j = i + 1; j < obj.length; j++) {
					if (obj[i].equals(obj[j])) {
						add = false;
						j = obj.length;
					}
				}

				if (add) {
					temp.add(obj[i]);
				}
				add = true;
			}
		}

		// need a loop in case obj[0] is null
		for (i = 0; i < obj.length; i++) {
			if (obj[i] != null)
				return temp.toArray((Object[]) Array.newInstance(obj[i]
						.getClass(), 0));
		}
		return null;
	}
}
