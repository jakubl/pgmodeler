/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2014 - Raphael Araújo e Silva <rkhaotix@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "modelwidget.h"
#include "sourcecodewidget.h"
#include "databasewidget.h"
#include "schemawidget.h"
#include "rolewidget.h"
#include "tablespacewidget.h"
#include "languagewidget.h"
#include "functionwidget.h"
#include "castwidget.h"
#include "conversionwidget.h"
#include "domainwidget.h"
#include "aggregatewidget.h"
#include "sequencewidget.h"
#include "operatorwidget.h"
#include "operatorfamilywidget.h"
#include "operatorclasswidget.h"
#include "typewidget.h"
#include "viewwidget.h"
#include "textboxwidget.h"
#include "columnwidget.h"
#include "constraintwidget.h"
#include "rulewidget.h"
#include "triggerwidget.h"
#include "indexwidget.h"
#include "relationshipwidget.h"
#include "tablewidget.h"
#include "taskprogresswidget.h"
#include "objectdepsrefswidget.h"
#include "objectrenamewidget.h"
#include "permissionwidget.h"
#include "collationwidget.h"
#include "extensionwidget.h"
#include "sqlappendwidget.h"
#include "tagwidget.h"

extern DatabaseWidget *database_wgt;
extern SchemaWidget *schema_wgt;
extern RoleWidget *role_wgt;
extern TablespaceWidget *tablespace_wgt;
extern LanguageWidget *language_wgt;
extern SourceCodeWidget *sourcecode_wgt;
extern FunctionWidget *function_wgt;
extern CastWidget *cast_wgt;
extern ConversionWidget *conversion_wgt;
extern DomainWidget *domain_wgt;
extern AggregateWidget *aggregate_wgt;
extern SequenceWidget *sequence_wgt;
extern OperatorWidget *operator_wgt;
extern OperatorFamilyWidget *opfamily_wgt;
extern OperatorClassWidget *opclass_wgt;
extern TypeWidget *type_wgt;
extern ViewWidget *view_wgt;
extern TextboxWidget *textbox_wgt;
extern ColumnWidget *column_wgt;
extern ConstraintWidget *constraint_wgt;
extern RuleWidget *rule_wgt;
extern TriggerWidget *trigger_wgt;
extern IndexWidget *index_wgt;
extern RelationshipWidget *relationship_wgt;
extern TableWidget *table_wgt;
extern CollationWidget *collation_wgt;
extern ExtensionWidget *extension_wgt;
extern TagWidget *tag_wgt;
extern TaskProgressWidget *task_prog_wgt;
extern ObjectDepsRefsWidget *deps_refs_wgt;
extern ObjectRenameWidget *objectrename_wgt;
extern PermissionWidget *permission_wgt;
extern SQLAppendWidget *sqlappend_wgt;

vector<BaseObject *> ModelWidget::copied_objects;
vector<BaseObject *> ModelWidget::cutted_objects;
bool ModelWidget::cut_operation=false;
ModelWidget *ModelWidget::src_model=nullptr;

const unsigned ModelWidget::BREAK_VERT_NINETY_DEGREES=0;
const unsigned ModelWidget::BREAK_HORIZ_NINETY_DEGREES=1;
const unsigned ModelWidget::BREAK_VERT_2NINETY_DEGREES=2;
const unsigned ModelWidget::BREAK_HORIZ_2NINETY_DEGREES=3;

ModelWidget::ModelWidget(QWidget *parent) : QWidget(parent)
{
	QFont font;
	QLabel *label=nullptr;
	QGridLayout *grid=nullptr;
	QAction *action=nullptr;
	QString str_ico, str_txt,
			rel_types_cod[]={"11", "1n", "nn", "dep", "gen" },
			rel_labels[]={"One to One (1-1)", "One to Many (1-n)", "Many to Many (n-n)", trUtf8("Copy"), trUtf8("Generalization")};
	ObjectType types[]={ OBJ_TABLE, OBJ_VIEW, OBJ_TEXTBOX, OBJ_RELATIONSHIP,
											 OBJ_CAST, OBJ_CONVERSION, OBJ_DOMAIN,
											 OBJ_FUNCTION, OBJ_AGGREGATE, OBJ_LANGUAGE,
											 OBJ_OPCLASS, OBJ_OPERATOR, OBJ_OPFAMILY,
											 OBJ_ROLE, OBJ_SCHEMA, OBJ_SEQUENCE, OBJ_TYPE,
											 OBJ_COLUMN, OBJ_CONSTRAINT, OBJ_RULE, OBJ_TRIGGER, OBJ_INDEX, OBJ_TABLESPACE,
                       OBJ_COLLATION, OBJ_EXTENSION, OBJ_TAG };
	unsigned i, obj_cnt=sizeof(types)/sizeof(ObjectType),
			rel_types_id[]={ BaseRelationship::RELATIONSHIP_11, BaseRelationship::RELATIONSHIP_1N,
												BaseRelationship::RELATIONSHIP_NN, BaseRelationship::RELATIONSHIP_DEP,
												BaseRelationship::RELATIONSHIP_GEN };

	current_zoom=1;
	modified=false;
	new_obj_type=BASE_OBJECT;

	//Generating a temporary file name for the model
	QTemporaryFile tmp_file;

	//Configuring the template mask which includes the full path to temporary dir
	tmp_file.setFileTemplate(GlobalAttributes::TEMPORARY_DIR + GlobalAttributes::DIR_SEPARATOR + QString("modelXXXXXX") + QString(".dbm"));
	tmp_file.open();
	tmp_filename=tmp_file.fileName();
	tmp_file.close();

	protected_model_frm=new QFrame(this);
	protected_model_frm->setGeometry(QRect(20, 10, 511, 48));
	protected_model_frm->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	protected_model_frm->setMinimumSize(QSize(0, 48));
	protected_model_frm->setFrameShape(QFrame::StyledPanel);
	protected_model_frm->setFrameShadow(QFrame::Raised);
	protected_model_frm->setVisible(false);

	label=new QLabel(protected_model_frm);
	label->setMinimumSize(QSize(32, 32));
	label->setMaximumSize(QSize(32, 32));
	label->setPixmap(QPixmap(Utf8String::create(":/icones/icones/msgbox_alerta.png")));

	grid=new QGridLayout;
	grid->addWidget(label, 0, 0, 1, 1);

	label=new QLabel(protected_model_frm);

	font.setPointSize(9);
	font.setBold(false);
	font.setItalic(false);
	font.setUnderline(false);
	font.setWeight(50);
	font.setStrikeOut(false);
	font.setKerning(true);
	label->setFont(font);
	label->setWordWrap(true);
	label->setText(trUtf8("<strong>ATTENTION:</strong> The database model is protected! Operations that could modify it are disabled!"));

	grid->addWidget(label, 0, 1, 1, 1);
	protected_model_frm->setLayout(grid);

	db_model=new DatabaseModel;
	op_list=new OperationList(db_model);
	scene=new ObjectsScene;
	scene->setSceneRect(QRectF(0,0,2000,2000));

	viewport=new QGraphicsView(scene);
	viewport->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	viewport->setRenderHint(QPainter::Antialiasing);
	viewport->setRenderHint(QPainter::TextAntialiasing);
	viewport->setRenderHint(QPainter::SmoothPixmapTransform);

	//Force the scene to be drawn from the left to right and from top to bottom
	viewport->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	viewport->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
	viewport->centerOn(0,0);
	this->applyZoom(1);

	grid=new QGridLayout;
	grid->addWidget(protected_model_frm, 0,0,1,1);
	grid->addWidget(viewport, 1,0,1,1);
	this->setLayout(grid);

	action_source_code=new QAction(QIcon(QString(":/icones/icones/codigosql.png")), trUtf8("Source"), this);
  action_source_code->setShortcut(QKeySequence(trUtf8("Alt+S")));
	action_source_code->setToolTip(trUtf8("Show object source code"));

	action_edit=new QAction(QIcon(QString(":/icones/icones/editar.png")), trUtf8("Properties"), this);
  action_edit->setShortcut(QKeySequence(trUtf8("Space")));
	action_edit->setToolTip(trUtf8("Edit the object properties"));

	action_protect=new QAction(QIcon(QString(":/icones/icones/bloqobjeto.png")), trUtf8("Protect"), this);
	action_unprotect=new QAction(QIcon(QString(":/icones/icones/desbloqobjeto.png")), trUtf8("Unprotect"), this);
	action_protect->setToolTip(trUtf8("Protects object(s) from modifications"));

	action_remove=new QAction(QIcon(QString(":/icones/icones/excluir.png")), trUtf8("Delete"), this);
  action_remove->setShortcut(QKeySequence(trUtf8("Del")));

	action_select_all=new QAction(QIcon(QString(":/icones/icones/seltodos.png")), trUtf8("Select all"), this);
  action_select_all->setShortcut(QKeySequence(trUtf8("Ctrl+A")));
	action_select_all->setToolTip(trUtf8("Selects all the graphical objects in the model"));

	action_convert_relnn=new QAction(QIcon(QString(":/icones/icones/convrelnn.png")), trUtf8("Convert"), this);

	action_copy=new QAction(QIcon(QString(":/icones/icones/copiar.png")), trUtf8("Copy"), this);
  action_copy->setShortcut(QKeySequence(trUtf8("Ctrl+C")));

	action_paste=new QAction(QIcon(QString(":/icones/icones/colar.png")), trUtf8("Paste"), this);
  action_paste->setShortcut(QKeySequence(trUtf8("Ctrl+V")));

	action_cut=new QAction(QIcon(QString(":/icones/icones/recortar.png")), trUtf8("Cut"), this);
  action_cut->setShortcut(QKeySequence(trUtf8("Ctrl+X")));

	action_deps_refs=new QAction(QIcon(QString(":/icones/icones/depsrefs.png")), trUtf8("Deps && Referrers"), this);

	action_new_object=new QAction(QIcon(QString(":/icones/icones/novoobjeto.png")), trUtf8("New"), this);
	action_new_object->setToolTip(trUtf8("Add a new object in the model"));

	action_quick_actions=new QAction(QIcon(QString(":/icones/icones/quickactions.png")), trUtf8("Quick"), this);
	action_quick_actions->setToolTip(trUtf8("Quick action for the selected object"));
	action_quick_actions->setMenu(&quick_actions_menu);

	action_rename=new QAction(QIcon(QString(":/icones/icones/rename.png")), trUtf8("Rename"), this);
  action_rename->setShortcut(QKeySequence(trUtf8("F2")));
	action_rename->setToolTip(trUtf8("Quick rename the object"));

	action_moveto_schema=new QAction(QIcon(QString(":/icones/icones/movetoschema.png")), trUtf8("Move to schema"), this);
	action_moveto_schema->setMenu(&schemas_menu);

  action_set_tag=new QAction(QIcon(QString(":/icones/icones/tag.png")), trUtf8("Set tag"), this);
  action_set_tag->setMenu(&tags_menu);

	action_edit_perms=new QAction(QIcon(QString(":/icones/icones/permission.png")), trUtf8("Edit permissions"), this);
  action_edit_perms->setShortcut(QKeySequence(trUtf8("Ctrl+E")));

	action_change_owner=new QAction(QIcon(QString(":/icones/icones/changeowner.png")), trUtf8("Change owner"), this);
	action_change_owner->setMenu(&owners_menu);

	action_sel_sch_children=new QAction(QIcon(QString(":/icones/icones/seltodos.png")), trUtf8("Select children"), this);
	action_highlight_object=new QAction(QIcon(QString(":/icones/icones/movimentado.png")), trUtf8("Highlight"), this);
	action_parent_rel=new QAction(QIcon(QString(":/icones/icones/relationship.png")), trUtf8("Open relationship"), this);

	action_append_sql=new QAction(QIcon(QString(":/icones/icones/sqlappend.png")), trUtf8("Append SQL"), this);
  action_append_sql->setShortcut(QKeySequence(trUtf8("Alt+Q")));

	action_create_seq_col=new QAction(QIcon(QString(":/icones/icones/sequence.png")), trUtf8("Create sequence"), this);
	action_break_rel_line=new QAction(QIcon(QString(":/icones/icones/breakrelline.png")), trUtf8("Break line"), this);

	action_remove_rel_points=new QAction(QIcon(QString(":/icones/icones/removepoints.png")), trUtf8("Remove points"), this);

	action=new QAction(QIcon(QString(":/icones/icones/breakline_90dv.png")), trUtf8("90° (vertical)"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(breakRelationshipLine(void)));
	action->setData(QVariant::fromValue<unsigned>(BREAK_VERT_NINETY_DEGREES));
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(QString(":/icones/icones/breakline_90dh.png")), trUtf8("90° (horizontal)"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(breakRelationshipLine(void)));
	action->setData(QVariant::fromValue<unsigned>(BREAK_HORIZ_NINETY_DEGREES));
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(QString(":/icones/icones/breakline_290dv.png")), trUtf8("90° + 90° (vertical)"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(breakRelationshipLine(void)));
	action->setData(QVariant::fromValue<unsigned>(BREAK_VERT_2NINETY_DEGREES));
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(QString(":/icones/icones/breakline_290dh.png")), trUtf8("90° + 90° (horizontal)"), this);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(breakRelationshipLine(void)));
	action->setData(QVariant::fromValue<unsigned>(BREAK_HORIZ_2NINETY_DEGREES));
	break_rel_menu.addAction(action);

	action_break_rel_line->setMenu(&break_rel_menu);

	//Alocatting the object creation actions
	for(i=0; i < obj_cnt; i++)
	{
		actions_new_objects[types[i]]=new QAction(QIcon(QString(":/icones/icones/") +
																							 BaseObject::getSchemaName(types[i]) + QString(".png")),
																				 BaseObject::getTypeName(types[i]), this);
		actions_new_objects[types[i]]->setData(QVariant(types[i]));
		connect(actions_new_objects[types[i]], SIGNAL(triggered(bool)), this, SLOT(addNewObject(void)));
	}

	//Creating the relationship submenu
	rels_menu=new QMenu(this);
	actions_new_objects[OBJ_RELATIONSHIP]->setMenu(rels_menu);

	for(i=0; i < 5; i++)
	{
		str_ico=BaseObject::getSchemaName(OBJ_RELATIONSHIP) + rel_types_cod[i] +  QString(".png");
		str_txt=rel_labels[i];

		action=new QAction(QIcon(QString(":/icones/icones/") + str_ico), str_txt, this);

		//Storing a unique identifier for the relationship type
		action->setData(QVariant(OBJ_RELATIONSHIP + rel_types_id[i]));

		connect(action, SIGNAL(triggered(bool)), this, SLOT(addNewObject(void)));
		rels_menu->addAction(action);
	}

	connect(action_source_code, SIGNAL(triggered(bool)), this, SLOT(showSourceCode(void)));
	connect(action_edit, SIGNAL(triggered(bool)),this,SLOT(editObject(void)));
	connect(action_protect, SIGNAL(triggered(bool)),this,SLOT(protectObject(void)));
	connect(action_unprotect, SIGNAL(triggered(bool)),this,SLOT(protectObject(void)));
	connect(action_remove, SIGNAL(triggered(bool)),this,SLOT(removeObjects(void)));
	connect(action_select_all, SIGNAL(triggered(bool)),this,SLOT(selectAllObjects(void)));
	connect(action_convert_relnn, SIGNAL(triggered(bool)), this, SLOT(convertRelationshipNN(void)));
	connect(action_deps_refs, SIGNAL(triggered(bool)), this, SLOT(showDependenciesReferences(void)));
	connect(action_copy, SIGNAL(triggered(bool)),this,SLOT(copyObjects(void)));
	connect(action_paste, SIGNAL(triggered(bool)),this,SLOT(pasteObjects(void)));
	connect(action_cut, SIGNAL(triggered(bool)),this,SLOT(cutObjects(void)));
	connect(action_rename, SIGNAL(triggered(bool)), this, SLOT(renameObject(void)));
	connect(action_edit_perms, SIGNAL(triggered(bool)), this, SLOT(editPermissions(void)));
	connect(action_sel_sch_children, SIGNAL(triggered(bool)), this, SLOT(selectSchemaChildren(void)));
	connect(action_highlight_object, SIGNAL(triggered(bool)), this, SLOT(highlightObject(void)));
	connect(action_parent_rel, SIGNAL(triggered(bool)), this, SLOT(editObject(void)));
	connect(action_append_sql, SIGNAL(triggered(bool)), this, SLOT(appendSQL(void)));
	connect(action_create_seq_col, SIGNAL(triggered(bool)), this, SLOT(createSequenceForColumn(void)));
	connect(action_remove_rel_points, SIGNAL(triggered(bool)), this, SLOT(removeRelationshipPoints(void)));

	connect(db_model, SIGNAL(s_objectAdded(BaseObject*)), this, SLOT(handleObjectAddition(BaseObject *)));
	connect(db_model, SIGNAL(s_objectRemoved(BaseObject*)), this, SLOT(handleObjectRemoval(BaseObject *)));

	connect(scene, SIGNAL(s_objectsMoved(bool)), this, SLOT(handleObjectsMovement(bool)));
	connect(scene, SIGNAL(s_objectModified(BaseGraphicObject*)), this, SLOT(handleObjectModification(BaseGraphicObject*)));
	connect(scene, SIGNAL(s_objectDoubleClicked(BaseGraphicObject*)), this, SLOT(handleObjectDoubleClick(BaseGraphicObject*)));
	connect(scene, SIGNAL(s_popupMenuRequested(BaseObject*)), this, SLOT(configureObjectMenu(BaseObject *)));
	connect(scene, SIGNAL(s_popupMenuRequested(void)), this, SLOT(showObjectMenu(void)));
	connect(scene, SIGNAL(s_objectSelected(BaseGraphicObject*,bool)), this, SLOT(configureObjectSelection(void)));

	viewport->installEventFilter(this);
	viewport->horizontalScrollBar()->installEventFilter(this);
	viewport->verticalScrollBar()->installEventFilter(this);
}

ModelWidget::~ModelWidget(void)
{
	op_list->removeOperations();
	db_model->destroyObjects();
	delete(viewport);
	delete(scene);
	delete(op_list);
	delete(db_model);
}

void ModelWidget::setModified(bool value)
{
	this->modified=value;
}

void ModelWidget::resizeEvent(QResizeEvent *)
{
	QRectF ret=scene->sceneRect();

	//Validating the width and height of the scene, resizing if the dimension is invalid
	if(viewport->rect().width() > ret.width())
		ret.setWidth(viewport->rect().width());

	if(viewport->rect().height() > ret.height())
		ret.setHeight(viewport->rect().height());

	scene->setSceneRect(ret);

	emit s_modelResized();
}

bool ModelWidget::eventFilter(QObject *object, QEvent *event)
{
	QWheelEvent *w_event=dynamic_cast<QWheelEvent *>(event);
	QKeyEvent *k_event=dynamic_cast<QKeyEvent *>(event);

	//Filters the Wheel event if it is raised by the viewport scrollbars
	if(event->type() == QEvent::Wheel && w_event->modifiers()==Qt::ControlModifier)
	{
		//Redirects the event to the wheelEvent() method of the model widget
		this->wheelEvent(w_event);
		return(true);
	}
	else if(event->type() == QEvent::KeyPress && k_event->modifiers()==Qt::AltModifier)
	{
		this->keyPressEvent(k_event);
		return(true);
	}
	else
		return(QWidget::eventFilter(object, event));
}

void ModelWidget::keyPressEvent(QKeyEvent *event)
{
	//Cancels the insertion action when ESC is pressed
	if(event->key()==Qt::Key_Escape)
	{
		this->cancelObjectAddition();
		scene->clearSelection();
	}
  else if((event->modifiers()==Qt::ControlModifier ||
          (event->modifiers()==(Qt::ControlModifier | Qt::ShiftModifier))) &&
          (event->key()==Qt::Key_Left || event->key()==Qt::Key_Right ||
           event->key()==Qt::Key_Down || event->key()==Qt::Key_Up))
  {
    int dx=0, dy=0, factor=1;

    if(event->key()==Qt::Key_Left)
      dx=-ObjectsScene::SCENE_MOVE_STEP;
    else if(event->key()==Qt::Key_Right)
      dx=ObjectsScene::SCENE_MOVE_STEP;
    else if(event->key()==Qt::Key_Up)
      dy=-ObjectsScene::SCENE_MOVE_STEP;
    else
      dy=ObjectsScene::SCENE_MOVE_STEP;

    if((event->modifiers() & Qt::ShiftModifier)==Qt::ShiftModifier)
      factor=4;

    viewport->horizontalScrollBar()->setValue(viewport->horizontalScrollBar()->value() + (dx * factor));
    viewport->verticalScrollBar()->setValue(viewport->verticalScrollBar()->value() + (dy * factor));
  }
}

void ModelWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->buttons()==Qt::LeftButton)
	{
		/* If the user is adding a graphical object, the left click will set the initial position and
		show the editing form related to the object type */
		if(new_obj_type==OBJ_TABLE || new_obj_type==OBJ_TEXTBOX || new_obj_type==OBJ_VIEW)
		{
      this->scene->enableRangeSelection(false);
			this->showObjectForm(new_obj_type, nullptr, nullptr, viewport->mapToScene(event->pos()));
			this->cancelObjectAddition();
      this->scene->enableRangeSelection(true);
		}
	}
}

void ModelWidget::wheelEvent(QWheelEvent * event)
{
	if(event->modifiers()==Qt::ControlModifier)
	{
		if(event->delta() < 0)
			this->applyZoom(this->current_zoom - ZOOM_INCREMENT);
		else
			this->applyZoom(this->current_zoom + ZOOM_INCREMENT);
  }
}

void ModelWidget::applyZoom(float zoom)
{
	//Aplica o zoom somente se este for válido
	if(zoom >= MINIMUM_ZOOM && zoom <= MAXIMUM_ZOOM)
	{
		viewport->resetTransform();
		viewport->scale(zoom, zoom);

		this->current_zoom=zoom;
		emit s_zoomModified(zoom);
	}
}

float ModelWidget::getCurrentZoom(void)
{
	return(current_zoom);
}

void ModelWidget::handleObjectAddition(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		ObjectType obj_type=graph_obj->getObjectType();
		QGraphicsItem *item=nullptr;

		switch(obj_type)
		{
			case OBJ_TABLE:
				item=new TableView(dynamic_cast<Table *>(graph_obj));
			break;

			case OBJ_VIEW:
				item=new GraphicalView(dynamic_cast<View *>(graph_obj));
			break;

			case OBJ_RELATIONSHIP:
			case BASE_RELATIONSHIP:
				item=new RelationshipView(dynamic_cast<BaseRelationship *>(graph_obj)); break;
			break;

			case OBJ_SCHEMA:
				item=new SchemaView(dynamic_cast<Schema *>(graph_obj)); break;
			break;

			default:
				item=new TextboxView(dynamic_cast<Textbox *>(graph_obj)); break;
			break;
		}

		scene->addItem(item);

		if(obj_type==OBJ_TABLE || obj_type==OBJ_VIEW)
			dynamic_cast<Schema *>(graph_obj->getSchema())->setModified(true);
	}

	this->modified=true;
}

void ModelWidget::addNewObject(void)
{
	QAction *action=dynamic_cast<QAction *>(sender());

	if(action)
	{
		BaseObject *parent_obj=nullptr;
		ObjectType obj_type=static_cast<ObjectType>(action->data().toUInt());

		/* If the user wants to add a table object or a object inside a schema
		uses as parent object the selected object, because the user only can add
		these types after select a table or schema, respectively */
		if(selected_objects.size()==1 &&
			 (TableObject::isTableObject(obj_type) ||
				selected_objects[0]->getObjectType()==OBJ_SCHEMA))
			parent_obj=selected_objects[0];

		//Creating a table or view inside a schema
		if(parent_obj && parent_obj->getObjectType()==OBJ_SCHEMA &&	 (obj_type==OBJ_TABLE || obj_type==OBJ_VIEW))
		{
			BaseObjectView *sch_graph=dynamic_cast<BaseObjectView *>(dynamic_cast<Schema *>(parent_obj)->getReceiverObject());
			QSizeF size = sch_graph->boundingRect().size();
			QPointF pos, menu_pos = viewport->mapToScene(this->mapFromGlobal(popup_menu.pos()));
			QRectF rect = QRectF(sch_graph->pos(), size);

			//Uses the popup menu position if it is inside the bounding rectangle
			if(rect.contains(menu_pos))
				pos=menu_pos;
			//Otherwise inserts the new object at the middle of bounding rect
			else
				pos=QPointF(sch_graph->pos().x() + (size.width()/2.0f),
										sch_graph->pos().y() + (size.height()/2.0f));

			this->showObjectForm(obj_type, nullptr, parent_obj, pos);
		}
		else if(obj_type!=OBJ_TABLE && obj_type!=OBJ_VIEW &&
						obj_type!=OBJ_TEXTBOX && obj_type <= BASE_TABLE)
			this->showObjectForm(obj_type, nullptr, parent_obj);
		else
		{
			//For the graphical object, changes the cursor icon until the user click on the model to show the editing form
			viewport->setCursor(QCursor(action->icon().pixmap(QSize(22,22)),0,0));
			this->new_obj_type=obj_type;
			this->enableModelActions(false);
		}
	}
}

void ModelWidget::handleObjectRemoval(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		scene->removeItem(dynamic_cast<QGraphicsItem *>(graph_obj->getReceiverObject()));

		//Updates the parent schema if the removed object were a table or view
		if(graph_obj->getSchema() &&
			 (graph_obj->getObjectType()==OBJ_TABLE || graph_obj->getObjectType()==OBJ_VIEW))
			dynamic_cast<Schema *>(graph_obj->getSchema())->setModified(true);
  }

	this->modified=true;
}

void ModelWidget::handleObjectDoubleClick(BaseGraphicObject *object)
{
	if(object)
		this->showObjectForm(object->getObjectType(), object, nullptr, object->getPosition());
	else
		this->showObjectForm(OBJ_DATABASE, db_model);
}

void ModelWidget::handleObjectsMovement(bool end_moviment)
{
	vector<BaseObject *> ::iterator itr, itr_end;
	BaseGraphicObject *obj=nullptr;

	itr=selected_objects.begin();
	itr_end=selected_objects.end();

	if(!end_moviment)
	{
		op_list->startOperationChain();

		while(itr!=itr_end)
		{
			obj=dynamic_cast<BaseGraphicObject *>(*itr);

			if(!dynamic_cast<BaseRelationship *>(obj) &&
				 !dynamic_cast<Schema *>(obj) &&
				 (obj && !obj->isProtected()))
				op_list->registerObject(obj, Operation::OBJECT_MOVED);

			itr++;
		}
	}
	else
	{
		vector<Schema *> schemas;

		while(itr!=itr_end)
		{
			obj=dynamic_cast<BaseGraphicObject *>(*itr);
			itr++;

			if(obj->getObjectType()==OBJ_TABLE || obj->getObjectType()==OBJ_VIEW)
			{
				Schema *schema=dynamic_cast<Schema *>(dynamic_cast<BaseTable *>(obj)->getSchema());

				//Update the schema if this isn't done yet
				if(std::find(schemas.begin(),schemas.end(),schema)==schemas.end())
				{
					schema->setModified(true);

					//Insert the updated schema to a list to avoid a second (unnecessary) update
					schemas.push_back(schema);
				}
			}
		}

		op_list->finishOperationChain();
		this->modified=true;

		emit s_objectsMoved();
	}
}

void ModelWidget::handleObjectModification(BaseGraphicObject *object)
{
	op_list->registerObject(object, Operation::OBJECT_MODIFIED);
	this->modified=true;

	if(object->getSchema())
		dynamic_cast<Schema *>(object->getSchema())->setModified(true);

	emit s_objectModified();
}

void ModelWidget::configureObjectSelection(void)
{
	QList<QGraphicsItem *> items=scene->selectedItems();
	BaseObjectView *item=nullptr;
	map<unsigned, QGraphicsItem *> objs_map;
	map<unsigned, QGraphicsItem *>::iterator itr;
	//deque<unsigned> sort_vect;

	selected_objects.clear();

	//Stores in a map each selected graphical object on the scene
	while(!items.isEmpty())
	{
		//Only store object that can be converted to BaseObjectView
		item=dynamic_cast<BaseObjectView *>(items.front());
		items.pop_front();

		if(item)
			objs_map[item->getSelectionOrder()]=item;
	}

	itr=objs_map.begin();
	while(itr!=objs_map.end())
	{
		item=dynamic_cast<BaseObjectView *>(itr->second);
		selected_objects.push_back(item->getSourceObject());
		itr++;
	}

	/* Case the new_obj_type is a value greater the BASE_TABLE indicates that the user
	(un)selected a object using some "Relationship" action */
	if(new_obj_type > BASE_TABLE)
	{
		unsigned count=selected_objects.size();
		ObjectType obj_type1, obj_type2;

		//If there is more than 2 object select cancel the operation
		if(count > 2 || count==0)
			this->cancelObjectAddition();
		else if(count >=1 && count <=2)
		{
			//Get the selected objects types
			obj_type1=selected_objects[0]->getObjectType();
			obj_type2=(count==2 ? selected_objects[1]->getObjectType() : BASE_OBJECT);

			//If there is only one selected object and this is a table, activates the relationship creation
			if(count==1 && obj_type1==OBJ_TABLE && new_obj_type > BASE_TABLE &&	 QApplication::keyboardModifiers()==0)
			{
				BaseGraphicObject *obj_graf=dynamic_cast<BaseGraphicObject *>(selected_objects[0]);
				BaseObjectView *objeto=dynamic_cast<BaseObjectView *>(obj_graf->getReceiverObject());

				scene->showRelationshipLine(true,
																	 QPointF(objeto->scenePos().x() + objeto->boundingRect().width()/2,
																					 objeto->scenePos().y() + objeto->boundingRect().height()/2));
			}
			//If the user has selected object that are not tables, cancel the operation
			else if(obj_type1!=OBJ_TABLE ||
							(obj_type2!=OBJ_TABLE && obj_type2!=BASE_OBJECT))
			{
				this->cancelObjectAddition();
			}

			/* Case there is only one selected object (table) and the SHIFT key is pressed too, creates a self-relationship.
				 Case there is two selected objects, create a relationship between them */
			else if((count==1 && obj_type1==OBJ_TABLE &&  QApplication::keyboardModifiers()==Qt::ShiftModifier) ||
							(count==2 && obj_type1==OBJ_TABLE && obj_type2==OBJ_TABLE))
			{
				this->showObjectForm(new_obj_type);

				//Cancels the operation after showing the relationship editing form
				scene->clearSelection();
				this->cancelObjectAddition();
			}
		}
	}
	else
		this->configurePopupMenu(selected_objects);
}

void ModelWidget::selectAllObjects(void)
{
	QPainterPath pth;
	pth.addRect(scene->sceneRect());
	scene->setSelectionArea(pth);
}

void ModelWidget::convertRelationshipNN(void)
{
	Relationship *rel=reinterpret_cast<Relationship *>(action_convert_relnn->data().value<void *>());

	if(rel)
	{
		//Converts only Many-to-Many relationship
		if(rel->getRelationshipType()==Relationship::RELATIONSHIP_NN)
		{
			msg_box.show(trUtf8("Confirmation"),
									 trUtf8("Do you really want to convert the relationship?"),
									 Messagebox::CONFIRM_ICON, Messagebox::YES_NO_BUTTONS);

			if(msg_box.result()==QDialog::Accepted)
			{
				unsigned op_count=0;

				try
				{
					Relationship *rel1=nullptr, *rel2=nullptr;
					Table *tab=nullptr, *tab_nn=nullptr,
							*src_tab=dynamic_cast<Table *>(rel->getTable(Relationship::SRC_TABLE)),
							*dst_tab=dynamic_cast<Table *>(rel->getTable(Relationship::DST_TABLE));
					Constraint *constr=nullptr, *aux_constr=nullptr;
					Column *col=nullptr;
					bool src_mand=true,	dst_mand=true;
					QString tab_name, xml_buf;
					QPointF pnt;
					unsigned i=1, idx, count, idx1, count1, x;
					vector<Constraint *> fks;

					op_count=op_list->getCurrentSize();

					//Stores the XML code definition for the table generated by the relationship
					tab_nn=rel->getReceiverTable();
					xml_buf=tab_nn->getCodeDefinition(SchemaParser::XML_DEFINITION);

					//Creates the table from the xml code
					XMLParser::restartParser();
					XMLParser::loadXMLBuffer(xml_buf);
					tab=db_model->createTable();
					tab_name=tab->getName();

					if(rel->isSelfRelationship())
					{
						//Copy the columns from the table generated by relationship to the newly created table
						count=tab_nn->getColumnCount();
						for(idx=0; idx < count; idx++)
						{
							col=new Column;
							(*col)=(*tab_nn->getColumn(idx));
							col->setParentTable(nullptr);
							tab->addColumn(col);
						}

						/* Copy the constraints from the table generated by relationship to the newly created table
						using the xml definition */
						count=tab_nn->getConstraintCount();
						for(idx=0; idx < count; idx++)
						{
							xml_buf=tab_nn->getConstraint(idx)->getCodeDefinition(SchemaParser::XML_DEFINITION,true);

							XMLParser::restartParser();
							XMLParser::loadXMLBuffer(xml_buf);
							constr=db_model->createConstraint(tab);
							tab->addConstraint(constr);
						}

						tab->getForeignKeys(fks, false, src_tab);
					}
					else
					{
						//Copies the relationship attributes to the created table
						count=rel->getAttributeCount();
						for(idx=0; idx < count; idx++)
						{
							col=new Column;
							(*col)=(*rel->getAttribute(idx));
							col->setParentTable(nullptr);
							tab->addColumn(col);
						}

						//Copies the relationship constraints to the created table
						count=rel->getConstraintCount();
						for(idx=0; idx < count; idx++)
						{
							constr=new Constraint;
							aux_constr=rel->getConstraint(idx);
							(*constr)=(*aux_constr);
							constr->removeColumns();
							constr->setParentTable(nullptr);

							for(x=Constraint::SOURCE_COLS; x <= Constraint::REFERENCED_COLS; x++)
							{
								count1=aux_constr->getColumnCount(x);
								for(idx1=0; idx1 < count1; idx1++)
								{
									col=tab->getColumn(aux_constr->getColumn(idx, x)->getName());
									if(col) constr->addColumn(col, x);
								}
							}
							tab->addConstraint(constr);
						}
					}

					//Renames the table if there is other with the same name on the model avoiding conflicts
					while(db_model->getObject(tab->getName(true), OBJ_TABLE))
					{
						tab->setName(tab_name + QString("_%1").arg(i));
						i++;
					}

					op_list->startOperationChain();

					//Removes the many-to-many relationship from the model
					op_list->registerObject(rel, Operation::OBJECT_REMOVED);
					db_model->removeObject(rel);

					//The default position for the table will be the middle point between the relationship participant tables
					pnt.setX((src_tab->getPosition().x() + dst_tab->getPosition().x())/2.0f);
					pnt.setY((src_tab->getPosition().y() + dst_tab->getPosition().y())/2.0f);
					tab->setPosition(pnt);

					//Adds the new table to the model
					db_model->addObject(tab);
					op_list->registerObject(tab, Operation::OBJECT_CREATED);

					if(rel->isSelfRelationship())
					{
						//For self relationships register the created foreign keys on the operation list
						while(!fks.empty())
						{
							op_list->registerObject(fks.back(), Operation::OBJECT_CREATED, -1, fks.back()->getParentTable());
							fks.pop_back();
						}
					}
					//If not self relationship creates two 1:n relationships
					else
					{
						//Creates a one-to-many relationship that links the source table of the many-to-many rel. to the created table
						rel1=new Relationship(Relationship::RELATIONSHIP_1N,
																	src_tab, tab, src_mand, false, true);
						db_model->addRelationship(rel1);
						op_list->registerObject(rel1, Operation::OBJECT_CREATED);

						//Creates a one-to-many relationship that links the destination table of the many-to-many rel. to the created table
						rel2=new Relationship(Relationship::RELATIONSHIP_1N,
																	dst_tab, tab, dst_mand, false, true);
						db_model->addRelationship(rel2);
						op_list->registerObject(rel2, Operation::OBJECT_CREATED);
					}

					op_list->finishOperationChain();
					emit s_objectCreated();
				}
				catch(Exception &e)
				{
					if(op_count < op_list->getCurrentSize())
					{
						unsigned qtd=op_list->getCurrentSize()-op_count;
						op_list->ignoreOperationChain(true);

						for(unsigned i=0; i < qtd; i++)
						{
							 op_list->undoOperation();
							 op_list->removeLastOperation();
						}

						op_list->ignoreOperationChain(false);
					}

					throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
				}
			}
		}
	}
}

void ModelWidget::loadModel(const QString &filename)
{
	try
	{
		connect(db_model, SIGNAL(s_objectLoaded(int,QString,unsigned)), task_prog_wgt, SLOT(updateProgress(int,QString,unsigned)));
		task_prog_wgt->setWindowTitle(trUtf8("Loading database model"));
		task_prog_wgt->show();

		db_model->loadModel(filename);
		this->filename=filename;
		this->adjustSceneSize();

		task_prog_wgt->close();
		disconnect(db_model, nullptr, task_prog_wgt, nullptr);

		protected_model_frm->setVisible(db_model->isProtected());
		this->modified=false;
	}
	catch(Exception &e)
	{
		task_prog_wgt->close();
		disconnect(db_model, nullptr, task_prog_wgt, nullptr);
		this->modified=false;
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::adjustSceneSize(void)
{
	QRectF scene_rect, objs_rect;
	bool align_objs, show_grid, show_delims;

	ObjectsScene::getGridOptions(show_grid, align_objs, show_delims);

	scene_rect=scene->sceneRect();
	objs_rect=scene->itemsBoundingRect();

	if(scene_rect.width() < objs_rect.left() + objs_rect.width())
		scene_rect.setWidth(objs_rect.left() + objs_rect.width());

	if(scene_rect.height() < objs_rect.top() + objs_rect.height())
		scene_rect.setHeight(objs_rect.top() + objs_rect.height());

	scene->setSceneRect(scene_rect);
	viewport->centerOn(0,0);

	if(align_objs)
		scene->alignObjectsToGrid();
}

void ModelWidget::printModel(QPrinter *printer, bool print_grid, bool print_page_nums)
{
	if(printer)
	{
		bool show_grid, align_objs, show_delims;
		unsigned page_cnt, page, h_page_cnt, v_page_cnt, h_pg_id, v_pg_id;
		vector<QRectF> pages;
		QRectF margins;
		QPrinter::PaperSize paper_size;
		QPrinter::Orientation orient;
		QSizeF page_size, custom_p_size;
		QPen pen;
		QFont font;
		QPointF top_left, top_right, bottom_left, bottom_right,
				h_top_mid, h_bottom_mid, v_left_mid, v_right_mid, dx, dy, dx1, dy1;

		//Make a backup of the current grid options
		ObjectsScene::getGridOptions(show_grid, align_objs, show_delims);

		//Reconfigure the grid options based upon the passed settings
		ObjectsScene::setGridOptions(print_grid, align_objs, false);

		scene->update();
		scene->clearSelection();

		//Get the page size based on the printer settings
		ObjectsScene::getPaperConfiguration(paper_size, orient, margins, custom_p_size);

		page_size=printer->paperSize(QPrinter::DevicePixel);

		if(paper_size!=QPrinter::Custom)
			page_size-=margins.size();
		else
			#warning "Custom page size bug (QTBUG-33645) workaround."
			printer->setPaperSize(QSizeF(page_size.height(), page_size.width()), QPrinter::DevicePixel);

		//Get the pages rect for printing
    pages=scene->getPagesForPrinting(page_size, margins.size(), h_page_cnt, v_page_cnt);

		//Creates a painter to draw the model directly on the printer
		QPainter painter(printer);
		painter.setRenderHint(QPainter::Antialiasing);
		font.setPointSizeF(7.5f);
		pen.setColor(QColor(120,120,120));
		pen.setWidthF(1.0f);

		//Calculates the auxiliary points to draw the page delimiter lines
		top_left.setX(0); top_left.setY(0);
		top_right.setX(page_size.width()); top_right.setY(0);
		bottom_left.setX(0); bottom_left.setY(page_size.height());
		bottom_right.setX(top_right.x()); bottom_right.setY(bottom_left.y());
		h_top_mid.setX(page_size.width()/2); h_top_mid.setY(0);
		h_bottom_mid.setX(h_top_mid.x()); h_bottom_mid.setY(bottom_right.y());
		v_left_mid.setX(top_left.x()); v_left_mid.setY(page_size.height()/2);
		v_right_mid.setX(top_right.x()); v_right_mid.setY(v_left_mid.y());

		dx.setX(margins.left());
		dx1.setX(margins.width());
		dy.setY(margins.top());
		dy1.setY(margins.height());

		page_cnt=pages.size();
		for(page=0, h_pg_id=0, v_pg_id=0; page < page_cnt; page++)
		{
			//Render the current page on the printer
			scene->render(&painter, QRect(), pages[page]);

			//Print the current page number is this option is marked
			if(print_page_nums)
			{
				painter.setPen(QColor(120,120,120));
				painter.drawText(-margins.left(), -margins.top(), QString("%1").arg(page+1));
			}

			//Print the guide lines at corners of the page
			painter.setPen(pen);
			if(h_pg_id==0 && v_pg_id==0)
			{
				painter.drawLine(top_left, top_left + dx);
				painter.drawLine(top_left, top_left + dy);
			}

			if(h_pg_id==h_page_cnt-1 && v_pg_id==0)
			{
				painter.drawLine(top_right, top_right - dx1);
				painter.drawLine(top_right, top_right + dy);
			}

			if(h_pg_id==0 && v_pg_id==v_page_cnt-1)
			{
				painter.drawLine(bottom_left, bottom_left + dx);
				painter.drawLine(bottom_left, bottom_left - dy1);
			}

			if(h_pg_id==h_page_cnt-1 && v_pg_id==v_page_cnt-1)
			{
				painter.drawLine(bottom_right, bottom_right - dx1);
				painter.drawLine(bottom_right, bottom_right - dy1);
			}

			if(h_pg_id >=1 && h_pg_id < h_page_cnt-1 && v_pg_id==0)
			{
				painter.drawLine(h_top_mid, h_top_mid - dx1);
				painter.drawLine(h_top_mid, h_top_mid + dx);
			}

			if(h_pg_id >=1 && h_pg_id < h_page_cnt-1 && v_pg_id==v_page_cnt-1)
			{
				painter.drawLine(h_bottom_mid, h_bottom_mid - dx1);
				painter.drawLine(h_bottom_mid, h_bottom_mid + dx);
			}

			if(v_pg_id >=1 && v_pg_id < v_page_cnt-1 && h_pg_id==0)
			{
				painter.drawLine(v_left_mid, v_left_mid - dy1);
				painter.drawLine(v_left_mid, v_left_mid + dy);
			}

			if(v_pg_id >=1 && v_pg_id < v_page_cnt-1 && h_pg_id==h_page_cnt-1)
			{
				painter.drawLine(v_right_mid, v_right_mid - dy1);
				painter.drawLine(v_right_mid, v_right_mid + dy);
			}

			h_pg_id++;

			if(h_pg_id==h_page_cnt)
			{
				h_pg_id=0;
				v_pg_id++;
			}

			if(page < page_cnt-1)
				printer->newPage();
		}

		//Restore the grid option backup
		ObjectsScene::setGridOptions(show_grid, align_objs, show_delims);
		scene->update();
	}
}

void ModelWidget::update(void)
{
	scene->update();
	QWidget::update();
}

void ModelWidget::saveModel(void)
{
	saveModel(this->filename);
}

void ModelWidget::saveModel(const QString &filename)
{
	try
	{
		connect(db_model, SIGNAL(s_objectLoaded(int,QString,unsigned)), task_prog_wgt, SLOT(updateProgress(int,QString,unsigned)));
		task_prog_wgt->setWindowTitle(trUtf8("Saving database model"));
		task_prog_wgt->show();

		db_model->saveModel(filename, SchemaParser::XML_DEFINITION);
		this->filename=filename;

		task_prog_wgt->close();
		disconnect(db_model, nullptr, task_prog_wgt, nullptr);
		this->modified=false;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

QString ModelWidget::getFilename(void)
{
	return(this->filename);
}

QString ModelWidget::getTempFilename(void)
{
	return(this->tmp_filename);
}

void ModelWidget::showObjectForm(ObjectType obj_type, BaseObject *object, BaseObject *parent_obj, QPointF pos)
{
	try
	{
		unsigned rel_type=0, res = QDialog::Rejected;
		Schema *sel_schema=dynamic_cast<Schema *>(parent_obj);

		/* Case the obj_type is greater than BASE_TABLE indicates that the object type is a
		 relationship. To get the specific relationship id (1-1, 1-n, n-n, gen, dep) is necessary
		 to subtract the OBJ_RELATIONSHIP from the obj_type parameter, the result will point
		 to the BaseRelationship::RELATIONSHIP_??? constant. */
		if(obj_type > BASE_TABLE)
		{
			rel_type=obj_type - OBJ_RELATIONSHIP;
			obj_type=OBJ_RELATIONSHIP;
		}

		if(obj_type!=OBJ_PERMISSION)
		{
			if(object && obj_type!=object->getObjectType())
				throw Exception(ERR_OPR_OBJ_INV_TYPE,__PRETTY_FUNCTION__,__FILE__,__LINE__);
			//If the user try to call the table object form without specify a parent object
			else if(!parent_obj && TableObject::isTableObject(obj_type))
				throw Exception(ERR_OPR_NOT_ALOC_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
		}

		if(object && dynamic_cast<BaseGraphicObject *>(object))
			pos=dynamic_cast<BaseGraphicObject *>(object)->getPosition();

		/* Raises an error if the user try to edit a reserverd object. The only exception is for "public" schema
		that can be edited only on its fill color an rectangle attributes */
		if(object && object->isSystemObject() && object->getName()!="public")
			throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
											.arg(object->getName()).arg(Utf8String::create(object->getTypeName())),
											ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

		switch(obj_type)
		{
			case OBJ_PERMISSION:
				permission_wgt->setAttributes(db_model, nullptr, object);
				permission_wgt->show();
			break;

			case OBJ_SCHEMA:
				schema_wgt->setAttributes(db_model, op_list, dynamic_cast<Schema *>(object));
				schema_wgt->show();
				res=(schema_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_ROLE:
				role_wgt->setAttributes(db_model, op_list, dynamic_cast<Role *>(object));
				role_wgt->show();
				res=(role_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_TABLESPACE:
				tablespace_wgt->setAttributes(db_model, op_list, dynamic_cast<Tablespace *>(object));
				tablespace_wgt->show();
				res=(tablespace_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_LANGUAGE:
				language_wgt->setAttributes(db_model, op_list, dynamic_cast<Language *>(object));
				language_wgt->show();
				res=(language_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_FUNCTION:
				function_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Function *>(object));
				function_wgt->show();
				res=(function_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_CAST:
				cast_wgt->setAttributes(db_model, op_list, dynamic_cast<Cast *>(object));
				cast_wgt->show();
				res=(cast_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_CONVERSION:
				conversion_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Conversion *>(object));
				conversion_wgt->show();
				res=(conversion_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_DOMAIN:
				domain_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Domain *>(object));
				domain_wgt->show();
				res=(domain_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_AGGREGATE:
				aggregate_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Aggregate *>(object));
				aggregate_wgt->show();
				res=(aggregate_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_SEQUENCE:
				sequence_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Sequence *>(object));
				sequence_wgt->show();
				res=(sequence_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_OPERATOR:
				operator_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Operator *>(object));
				operator_wgt->show();
				res=(operator_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_OPFAMILY:
				opfamily_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<OperatorFamily *>(object));
				opfamily_wgt->show();
				res=(opfamily_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_OPCLASS:
				opclass_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<OperatorClass *>(object));
				opclass_wgt->show();
				res=(opclass_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_TYPE:
				type_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Type *>(object));
				type_wgt->show();
				res=(type_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_VIEW:
				View *view;
				view=dynamic_cast<View *>(object);
				view_wgt->setAttributes(db_model, op_list, sel_schema, view, pos.x(), pos.y());
				view_wgt->show();
				res=(view_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_TEXTBOX:
				Textbox *txtbox;
				txtbox=dynamic_cast<Textbox *>(object);
				textbox_wgt->setAttributes(db_model, op_list, txtbox, pos.x(), pos.y());
				textbox_wgt->show();
				res=(textbox_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_COLUMN:
				Column *col;
				col=dynamic_cast<Column *>(object);
				column_wgt->setAttributes(db_model, parent_obj, op_list, col);
				column_wgt->show();
				res=(column_wgt->result()==QDialog::Accepted);

				if(res==QDialog::Accepted)
				{
					if(col)
						db_model->validateRelationships(col, dynamic_cast<Table *>(parent_obj));
					else
						db_model->validateRelationships();
				}
			break;

			case OBJ_CONSTRAINT:
				Constraint *constr;
				constr=dynamic_cast<Constraint *>(object);
				constraint_wgt->setAttributes(db_model, parent_obj, op_list, constr);
				constraint_wgt->show();
				res=(constraint_wgt->result()==QDialog::Accepted);

				if(res==QDialog::Accepted)
				{
					if(constr && parent_obj->getObjectType()==OBJ_TABLE)
						db_model->validateRelationships(constr, dynamic_cast<Table *>(parent_obj));
					else
						db_model->validateRelationships();
				}
			break;

			case OBJ_RULE:
				rule_wgt->setAttributes(db_model, dynamic_cast<BaseTable *>(parent_obj), op_list, dynamic_cast<Rule *>(object));
				rule_wgt->show();
				res=(rule_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_TRIGGER:
				trigger_wgt->setAttributes(db_model, dynamic_cast<BaseTable *>(parent_obj), op_list, dynamic_cast<Trigger *>(object));
				trigger_wgt->show();
				res=(trigger_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_INDEX:
				index_wgt->setAttributes(db_model, dynamic_cast<Table *>(parent_obj), op_list, dynamic_cast<Index *>(object));
				index_wgt->show();
				res=(index_wgt->result()==QDialog::Accepted);
			break;

			case BASE_RELATIONSHIP:
			case OBJ_RELATIONSHIP:
				if(!object && rel_type > 0 &&
					 selected_objects.size() > 0 &&
					 selected_objects[0]->getObjectType()==OBJ_TABLE)
				{
					Table *tab1=dynamic_cast<Table *>(selected_objects[0]),
							*tab2=(selected_objects.size()==2 ?
											 dynamic_cast<Table *>(selected_objects[1]) : tab1);
					relationship_wgt->setAttributes(db_model, op_list, tab1, tab2, rel_type);
				}
				else
					relationship_wgt->setAttributes(db_model, op_list, dynamic_cast<BaseRelationship *>(object));

				relationship_wgt->show();
				scene->clearSelection();
				res=(relationship_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_TABLE:
				table_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Table *>(object), pos.x(), pos.y());
				table_wgt->show();
				res=(table_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_COLLATION:
				collation_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Collation *>(object));
				collation_wgt->show();
				res=(collation_wgt->result()==QDialog::Accepted);
			break;

			case OBJ_EXTENSION:
				extension_wgt->setAttributes(db_model, op_list, sel_schema, dynamic_cast<Extension *>(object));
				extension_wgt->show();
        res=(extension_wgt->result()==QDialog::Accepted);
			break;

      case OBJ_TAG:
        tag_wgt->setAttributes(db_model, op_list, dynamic_cast<Tag *>(object));
        tag_wgt->show();
        res=(tag_wgt->result()==QDialog::Accepted);
      break;

			default:
			case OBJ_DATABASE:
				database_wgt->setAttributes(db_model);
				database_wgt->show();
				res=(database_wgt->result()==QDialog::Accepted);
			break;
		}

		if(!this->modified && res==QDialog::Accepted)
		{
			this->modified=true;
			this->db_model->setInvalidated(true);
		}

		this->setFocus();
	}
	catch(Exception &e)
	{
		msg_box.show(e);
	}
}

void ModelWidget::showDependenciesReferences(void)
{
	QAction *obj_sender=dynamic_cast<QAction *>(sender());

	if(obj_sender)
	{
		BaseObject *object=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

		if(object)
		{
			deps_refs_wgt->setAttributes(this->db_model, object);
			deps_refs_wgt->show();
		}
	}
}

void ModelWidget::showSourceCode(void)
{
	QAction *obj_sender=dynamic_cast<QAction *>(sender());

	if(obj_sender)
	{
		BaseObject *objeto=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

		if(objeto)
		{
			sourcecode_wgt->setAttributes(this->db_model, objeto);
			sourcecode_wgt->show();
		}
	}
}

void ModelWidget::cancelObjectAddition(void)
{
	//Reset the new object type to a invalid one forcing the user to select a correct type again
	new_obj_type=BASE_OBJECT;

	//Restore the cursor icon
	viewport->setCursor(QCursor(Qt::ArrowCursor));

	//Hide the line that simulates the relationship creation
	scene->showRelationshipLine(false);

	this->configurePopupMenu(this->selected_objects);
}

void ModelWidget::renameObject(void)
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *obj=reinterpret_cast<BaseObject *>(act->data().value<void *>());

	if(obj->isSystemObject())
		throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
										.arg(obj->getName()).arg(Utf8String::create(obj->getTypeName())),
										ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	objectrename_wgt->setAttributes(obj, this->db_model, this->op_list);
	objectrename_wgt->exec();

	if(objectrename_wgt->result()==QDialog::Accepted)
	{
		this->modified=true;
		emit s_objectModified();
	}
}

void ModelWidget::moveToSchema(void)
{
	QAction *act=dynamic_cast<QAction *>(sender());
	Schema *schema=dynamic_cast<Schema *>(reinterpret_cast<BaseObject *>(act->data().value<void *>())),
			*prev_schema=dynamic_cast<Schema *>(selected_objects[0]->getSchema());
	BaseGraphicObject *obj_graph=nullptr;

	try
	{
		op_list->registerObject(selected_objects[0], Operation::OBJECT_MODIFIED, -1);
		selected_objects[0]->setSchema(schema);
		obj_graph=dynamic_cast<BaseGraphicObject *>(selected_objects[0]);

		if(obj_graph)
		{
      SchemaView *dst_schema=dynamic_cast<SchemaView *>(schema->getReceiverObject());
      QPointF p;

      if(dst_schema->isVisible())
      {
        p.setX(dst_schema->pos().x());
        p.setY(dst_schema->pos().y() + dst_schema->boundingRect().height() + BaseObjectView::VERT_SPACING);
        dynamic_cast<BaseObjectView *>(obj_graph->getReceiverObject())->setPos(p);
      }

      obj_graph->setModified(true);
      schema->setModified(true);
      prev_schema->setModified(true);
		}

		emit s_objectModified();
	}
	catch(Exception &e)
	{
		op_list->removeLastOperation();
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::changeOwner(void)
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *owner=reinterpret_cast<BaseObject *>(act->data().value<void *>()),
			*obj=(!selected_objects.empty() ? selected_objects[0] : this->db_model);

	if(selected_objects[0]->isSystemObject())
		throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
										.arg(selected_objects[0]->getName()).arg(Utf8String::create(selected_objects[0]->getTypeName())),
										ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	try
	{
		if(obj->getObjectType()!=OBJ_DATABASE)
			op_list->registerObject(obj, Operation::OBJECT_MODIFIED, -1);

		obj->setOwner(owner);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		if(obj->getObjectType()!=OBJ_DATABASE)
			op_list->removeLastOperation();

		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
  }
}

void ModelWidget::setTag(void)
{
  QAction *act=dynamic_cast<QAction *>(sender());
  BaseObject *tag=reinterpret_cast<BaseObject *>(act->data().value<void *>()),
      *obj=(!selected_objects.empty() ? selected_objects[0] : this->db_model);
  BaseTable *tab=dynamic_cast<BaseTable *>(obj);


  try
  {
    op_list->registerObject(obj, Operation::OBJECT_MODIFIED, -1);

    tab->setTag(dynamic_cast<Tag *>(tag));
    tab->setModified(true);

    emit s_objectModified();
  }
  catch(Exception &e)
  {
    op_list->removeLastOperation();
    throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
  }
}

void ModelWidget::editPermissions(void)
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *obj=reinterpret_cast<BaseObject *>(act->data().value<void *>());

	permission_wgt->setAttributes(this->db_model, nullptr, obj);
	permission_wgt->show();
	this->setModified(true);
}

void ModelWidget::editObject(void)
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	TableObject *tab_obj=nullptr;
	BaseObject *object=nullptr;

	/* Workaround: To permit the object edition via double click on the ModelObjectWidget the sender
	is configured as the edit action of the model widget */
	if(!obj_sender)
		obj_sender=action_edit;

	object=reinterpret_cast<BaseObject *>(dynamic_cast<QAction *>(obj_sender)->data().value<void *>());
	tab_obj=dynamic_cast<TableObject *>(object);

	if(object)
		showObjectForm(object->getObjectType(), object,
										 (tab_obj ? tab_obj->getParentTable() : nullptr));
}

void ModelWidget::selectSchemaChildren(void)
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	Schema *schema=nullptr;

	schema=dynamic_cast<Schema *>(
						reinterpret_cast<BaseObject *>(
							dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	scene->clearSelection();

	dynamic_cast<SchemaView *>(
				dynamic_cast<BaseObjectView *>(schema->getReceiverObject()))->selectChildren();
}

void ModelWidget::protectObject(void)
{
	try
	{
		QObject *obj_sender=sender();
		ObjectType obj_type;
		TableObject *tab_obj=nullptr;
		BaseObject *object=nullptr;
		BaseGraphicObject *graph_obj=nullptr;
		bool protect=false;
		vector<BaseObject *>::iterator itr, itr_end;

		scene->blockSignals(true);

		if(this->selected_objects.size()==1)
		{
			tab_obj=dynamic_cast<TableObject *>(this->selected_objects[0]);
			graph_obj=dynamic_cast<BaseGraphicObject *>(this->selected_objects[0]);

			if(graph_obj)
				graph_obj->setProtected(!graph_obj->isProtected());
			else if(tab_obj)
			{
				tab_obj->setProtected(!tab_obj->isProtected());

				//Force the update of the parent table
				dynamic_cast<Table *>(tab_obj->getParentTable())->setModified(true);
			}
			else
			{
				//Raise an error if the user try to modify a reserved object protection
				if(this->selected_objects[0]->isSystemObject())
					throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
													.arg(selected_objects[0]->getName()).arg(Utf8String::create(selected_objects[0]->getTypeName())),
													ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

				this->selected_objects[0]->setProtected(!this->selected_objects[0]->isProtected());
			}
		}
		//Protects the whole model if there is no selected object
		else if(this->selected_objects.empty())
		{
			if(obj_sender==action_protect || obj_sender==action_unprotect)
				db_model->setProtected(!db_model->isProtected());
		}
		//If there is more than one selected object, make a batch protection/unprotection
		else
		{
			itr=this->selected_objects.begin();
			itr_end=this->selected_objects.end();
			protect=(!this->selected_objects[0]->isProtected());

			while(itr!=itr_end)
			{
				object=(*itr);
				graph_obj=dynamic_cast<BaseGraphicObject *>(object);
				itr++;

				obj_type=object->getObjectType();

				if(object->isSystemObject())
					throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
													.arg(object->getName()).arg(Utf8String::create(object->getTypeName())),
													ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
				else if(obj_type==OBJ_COLUMN || obj_type==OBJ_CONSTRAINT)
				{
					tab_obj=dynamic_cast<TableObject *>(object);

					if(tab_obj->isAddedByRelationship())
					{
						throw Exception(QString(Exception::getErrorMessage(ERR_OPR_REL_INCL_OBJECT))
														.arg(object->getName()).arg(object->getTypeName()),
														ERR_OPR_REL_INCL_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
					}
				}

				object->setProtected(protect);
			}
		}

		protected_model_frm->setVisible(db_model->isProtected());
		scene->blockSignals(false);
		scene->clearSelection();

		emit s_objectModified();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::cutObjects(void)
{
	/* Store the source model as 'this'. This attribute is used on the paste method
	to remove the selected object and updated the source model */
	ModelWidget::src_model=this;

	//Set the flag indicating that a cut operation started
	ModelWidget::cut_operation=true;
	this->copyObjects();
}

void ModelWidget::copyObjects(void)
{
	map<unsigned, BaseObject *> objs_map;
	map<unsigned, BaseObject *>::iterator obj_itr;
	vector<BaseObject *>::iterator itr, itr_end;
	vector<BaseObject *> deps;
	BaseObject *object=nullptr;
	TableObject *tab_obj=nullptr;
	Table *table=nullptr;
	Constraint *constr=nullptr;
	ObjectType types[]={ OBJ_TRIGGER, OBJ_INDEX, OBJ_CONSTRAINT };
	unsigned i, type_id, count;

	if(selected_objects.size()==1)
	{
		//Raise an error if the user try to copy a reserved object
		if(selected_objects[0]->isSystemObject())
			throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
											.arg(selected_objects[0]->getName()).arg(Utf8String::create(selected_objects[0]->getTypeName())),
											ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
	}

	//Ask for confirmation to copy the dependencies of the object(s)
	msg_box.show(trUtf8("Confirmation"),
								trUtf8("Also copy all dependencies of selected objects? This minimizes the breakdown of references when copied objects are pasted into another model."),
								Messagebox::CONFIRM_ICON, Messagebox::YES_NO_BUTTONS);

	/* When in cut operation is necessary to store the selected objects in a separeted list
	in order to correclty cut (remove) the object on the source model */
	if(ModelWidget::cut_operation)
		cutted_objects=selected_objects;

	itr=selected_objects.begin();
	itr_end=selected_objects.end();

	while(itr!=itr_end)
	{
		object=(*itr);

		//Table-view relationships and FK relationship aren't copied since they are created automatically when pasting the tables/views
		if(object->getObjectType()!=BASE_RELATIONSHIP)
		{
			//Get the object dependencies if the user confirmed this situation
			db_model->getObjectDependecies(object, deps, msg_box.result()==QDialog::Accepted);

			/* Copying the special objects (which references columns added by relationship) in order
			to be correclty created when pasted */
			if(object->getObjectType()==OBJ_TABLE)
			{
				table=dynamic_cast<Table *>(object);

				for(type_id=0; type_id < 3; type_id++)
				{
					count=table->getObjectCount(types[type_id]);

					for(i=0; i < count; i++)
					{
						tab_obj=dynamic_cast<TableObject *>(table->getObject(i, types[type_id]));
						constr=dynamic_cast<Constraint *>(tab_obj);

						/* The object is only inserted at the list when it was not included by relationship but references
						columns added by relationship. Case the object is a constraint, it cannot be a primary key because
						this type of constraint is treated separetely by relationships */
						if(!tab_obj->isAddedByRelationship() &&
							 ((constr &&
								 (constr->getConstraintType()==ConstraintType::foreign_key ||
									(constr->getConstraintType()==ConstraintType::unique &&
									 constr->isReferRelationshipAddedColumn()))) ||
								(types[type_id]==OBJ_TRIGGER && dynamic_cast<Trigger *>(tab_obj)->isReferRelationshipAddedColumn()) ||
								(types[type_id]==OBJ_INDEX && dynamic_cast<Index *>(tab_obj)->isReferRelationshipAddedColumn())))
							deps.push_back(tab_obj);
					}
				}
			}
		}
		itr++;
	}

	itr=deps.begin();
	itr_end=deps.end();


	//Storing the objects ids in a auxiliary vector
	while(itr!=itr_end)
	{
		object=(*itr);
		//objs_id.push_back(object->getObjectId());
		objs_map[object->getObjectId()]=object;
		itr++;
	}

	obj_itr=objs_map.begin();
	while(obj_itr!=objs_map.end())
	{
		object=obj_itr->second;

		//Reserved object aren't copied
		if(!object->isSystemObject())
			copied_objects.push_back(object);

		obj_itr++;
	}
}

void ModelWidget::pasteObjects(void)
{
	map<BaseObject *, QString> xml_objs;
	BaseTable *orig_parent_tab=nullptr;
	vector<BaseObject *>::iterator itr, itr_end;
	map<BaseObject *, QString> orig_obj_names;
	BaseObject *object=nullptr, *aux_object=nullptr;
	TableObject *tab_obj=nullptr;
	Table *sel_table=nullptr;
	View *sel_view=nullptr;
	BaseTable *parent=nullptr;
	Function *func=nullptr;
	Constraint *constr=nullptr;
	Operator *oper=nullptr;
	QString aux_name, copy_obj_name;
	ObjectType obj_type;
	Exception error;
	unsigned idx=0, pos=0;

	task_prog_wgt->setWindowTitle(trUtf8("Pasting objects..."));
	task_prog_wgt->show();

	itr=copied_objects.begin();
	itr_end=copied_objects.end();

	/* If there is only one object selected, check if its a table or view.
	Because if the user try to paste a table object the receiver object (selected)
	must be a table or view */
	if(selected_objects.size()==1)
	{
		sel_table=dynamic_cast<Table *>(selected_objects[0]);
		sel_view=dynamic_cast<View *>(selected_objects[0]);
	}

	while(itr!=itr_end)
	{
		object=(*itr);
		obj_type=object->getObjectType();
		tab_obj=dynamic_cast<TableObject *>(object);
		itr++;
		pos++;
		task_prog_wgt->updateProgress((pos/static_cast<float>(copied_objects.size()))*100,
																	trUtf8("Validating object: %1 (%2)").arg(object->getName())
																	.arg(object->getTypeName()),
																	object->getObjectType());

		if(!tab_obj || ((sel_table || sel_view) && tab_obj))
		{
			/* The first validation is to check if the object to be pasted does not conflict
			with any other object of the same type on the model */

			if(obj_type==OBJ_FUNCTION)
			{
				dynamic_cast<Function *>(object)->createSignature(true);
				aux_name=dynamic_cast<Function *>(object)->getSignature();
			}
			else if(obj_type==OBJ_OPERATOR)
				aux_name=dynamic_cast<Operator *>(object)->getSignature();
			else
				aux_name=object->getName(true);

			if(!tab_obj)
				//Try to find the object on the model
				aux_object=db_model->getObject(aux_name, obj_type);
			else
			{
				if(sel_view && (obj_type==OBJ_TRIGGER || obj_type==OBJ_RULE))
					aux_object=sel_view->getObject(aux_name, obj_type);
				else if(sel_table)
					aux_object=sel_table->getObject(aux_name, obj_type);
			}

			/* The second validation is to check, when the object is found on the model, if the XML code of the found object
			 and the object to be pasted are different. When the XML defintion are the same the object isn't pasted because
			 the found object can be used as substitute of the object to be pasted. This operation is not applied to graphical
			 objects because they are ALWAYS pasted on the model. The only exception is that the below code is executed when the
			 found object is the same as the copied object (this means that user is copying and pasting the object at the same database) */
			if(tab_obj ||
				 (aux_object &&
					(dynamic_cast<BaseGraphicObject *>(object) ||
					 (aux_object->getDatabase()==object->getDatabase()) ||
						 (aux_object->getCodeDefinition(SchemaParser::SchemaParser::XML_DEFINITION) !=
							object->getCodeDefinition(SchemaParser::SchemaParser::XML_DEFINITION)))))
			{
				//Resolving name conflicts
				if(obj_type!=OBJ_CAST)
				{
					func=nullptr; oper=nullptr;
					aux_name.clear();
					idx=0;

					//Store the orignal object name on a map
					orig_obj_names[object]=object->getName();

					do
					{
						//Creates an name suffix assigned to the object to be pasted in order to resolve conflicts
						if(idx > 0)
						{
							if(obj_type==OBJ_OPERATOR)
								aux_name=QString("").leftJustified(idx,'?');
							else
								aux_name=QString("_cp%1").arg(idx);
						}
						idx++;

						/* For each object type as follow configures the name and the suffix and store them on the
						'copy_obj_name' variable. This string is used to check if there are objects with the same name
						on model. While the 'copy_obj_name' conflicts with other objects (of same type) this validation is made */
						if(obj_type==OBJ_FUNCTION)
						{
							func=dynamic_cast<Function *>(object);
							func->setName(orig_obj_names[object] + aux_name);
							copy_obj_name=func->getSignature();
							func->setName(orig_obj_names[object]);
						}
						else if(obj_type==OBJ_OPERATOR)
						{
							oper=dynamic_cast<Operator *>(object);
							oper->setName(orig_obj_names[object] + aux_name);
							copy_obj_name=oper->getSignature();
							oper->setName(orig_obj_names[object]);
						}
						else
						{
							object->setName(orig_obj_names[object] + aux_name);
							copy_obj_name=object->getName(true);
							object->setName(orig_obj_names[object]);
						}
					}
					while((!tab_obj && db_model->getObject(copy_obj_name, obj_type)) ||
								(tab_obj && sel_table && sel_table->getObject(copy_obj_name, obj_type)) ||
								(tab_obj && sel_view  && (obj_type==OBJ_TRIGGER || obj_type==OBJ_RULE) &&
								 sel_view->getObject(copy_obj_name, obj_type)));

					//Sets the new object name concatenating the suffix to the original name
					object->setName(orig_obj_names[object] + aux_name);
				}
			}
		}
	}

	/* The third step is get the XML code definition of the copied objects, is
	with the xml code that the copied object are created and inserted on the model */
	itr=copied_objects.begin();
	itr_end=copied_objects.end();
	pos=0;
	while(itr!=itr_end)
	{
		object=(*itr);
		tab_obj=dynamic_cast<TableObject *>(object);
		itr++;

		pos++;
		task_prog_wgt->updateProgress((pos/static_cast<float>(copied_objects.size()))*100,
																	trUtf8("Generating XML code of object: %1 (%2)").arg(object->getName())
																	.arg(object->getTypeName()),
																	object->getObjectType());

		//Store the original parent table of the object
		if(tab_obj && (sel_table || sel_view))
		{
			if(sel_table)
				parent=sel_table;
			else
				parent=sel_view;

			/* Only generates the XML for a table object when the selected receiver object
			is a table or is a view and the current object is a trigger or rule (because
			view's only accepts this two types) */
			if(sel_table ||
				 (sel_view && (tab_obj->getObjectType()==OBJ_TRIGGER ||
											 tab_obj->getObjectType()==OBJ_RULE)))
			{
				//Backups the original parent table
				orig_parent_tab=tab_obj->getParentTable();

				//Set the parent table as the selected table/view
				tab_obj->setParentTable(parent);

				//Generates the XML code with the new parent table
				xml_objs[object]=object->getCodeDefinition(SchemaParser::XML_DEFINITION);

				//Restore the original parent table
				tab_obj->setParentTable(orig_parent_tab);
			}
		}
		else if(!tab_obj)
			//Stores the XML definition on a xml buffer map
			xml_objs[object]=object->getCodeDefinition(SchemaParser::XML_DEFINITION);
	}

	//The fourth step is the restoration of original names of the copied objects
	itr=copied_objects.begin();
	itr_end=copied_objects.end();

	while(itr!=itr_end)
	{
		object=(*itr);
		itr++;

		if(orig_obj_names[object].count() && obj_type!=OBJ_CAST)
			object->setName(orig_obj_names[object]);
	}

	//The last step is create the object from the stored xmls
	itr=copied_objects.begin();
	itr_end=copied_objects.end();
	pos=0;

	op_list->startOperationChain();

	while(itr!=itr_end)
	{
		if(xml_objs.count(*itr))
		{
			XMLParser::restartParser();
			XMLParser::loadXMLBuffer(xml_objs[*itr]);

			try
			{
				//Creates the object from the XML
				object=db_model->createObject(db_model->getObjectType(XMLParser::getElementName()));
				tab_obj=dynamic_cast<TableObject *>(object);
				constr=dynamic_cast<Constraint *>(tab_obj);

				pos++;
				task_prog_wgt->updateProgress((pos/static_cast<float>(copied_objects.size()))*100,
																			trUtf8("Pasting object: %1 (%2)").arg(object->getName())
																			.arg(object->getTypeName()),
																			object->getObjectType());

				/* Once created, the object is added on the model, except for relationships and table objects
			because they are inserted automatically */
				if(object && !tab_obj && !dynamic_cast<Relationship *>(object))
					db_model->addObject(object);

				//Special case for table objects
				if(tab_obj)
				{
					if(sel_table &&
						 (tab_obj->getObjectType()==OBJ_COLUMN ||	tab_obj->getObjectType()==OBJ_RULE))
					{
						sel_table->addObject(tab_obj);
						sel_table->setModified(true);
					}
					else if(sel_view && tab_obj->getObjectType()==OBJ_RULE)
					{
						sel_view->addObject(tab_obj);
						sel_view->setModified(true);
					}

					//Updates the fk relationships if the constraint is a foreign-key
					if(constr && constr->getConstraintType()==ConstraintType::foreign_key)
						db_model->updateTableFKRelationships(dynamic_cast<Table *>(tab_obj->getParentTable()));

					op_list->registerObject(tab_obj, Operation::OBJECT_CREATED, -1, tab_obj->getParentTable());
				}
				else
					op_list->registerObject(object, Operation::OBJECT_CREATED);
			}
			catch(Exception &e)
			{
				error=e;
			}
		}

		itr++;
	}
	op_list->finishOperationChain();

	//Validates the relationships to reflect any modification on the tables structures and not propagated columns
	db_model->validateRelationships();

	this->adjustSceneSize();
	task_prog_wgt->close();

	//If some error occur during the process show it to the user
	if(error.getErrorType()!=ERR_CUSTOM)
		msg_box.show(error,
								 trUtf8("Not all objects were pasted to the model due to errors returned during the process! Refer to error stack for more details!"),
								 Messagebox::ALERT_ICON);

	if(!ModelWidget::cut_operation)
	{
		copied_objects.clear();
		emit s_objectCreated();
	}
	//If its a cut operatoin
	else
	{
		//Remove the objects from the source model
		ModelWidget::src_model->selected_objects=ModelWidget::cutted_objects;
		ModelWidget::src_model->removeObjects();

		//Uncheck the cut operation flag
		ModelWidget::cut_operation=false;

		copied_objects.clear();
		cutted_objects.clear();
		if(this!=ModelWidget::src_model)
			ModelWidget::src_model->configurePopupMenu();

		ModelWidget::src_model=nullptr;
	}

	this->configurePopupMenu();
	this->modified=true;
}

void ModelWidget::removeObjects(void)
{
	int obj_idx=-1;
	unsigned count, op_count=0;
	Table *aux_table=nullptr;
	BaseTable *table=nullptr, *src_table=nullptr, *dst_table=nullptr;
	BaseRelationship *rel=nullptr;
	TableObject *tab_obj=nullptr;
	ObjectType obj_type;
	BaseObject *object=nullptr;
	vector<BaseObject *>::iterator itr, itr_end;
	vector<Constraint *> constrs;
	map<unsigned, BaseObject *> objs_map;
	map<unsigned, BaseObject *>::reverse_iterator ritr, ritr_end;
	QAction *obj_sender=dynamic_cast<QAction *>(sender());

	if(obj_sender)
		object=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

	if(!selected_objects.empty() || object)
	{
		//If the removal is not due to a cut operation, ask for permission to remove the objects
		if(!ModelWidget::cut_operation)
		{
			if(selected_objects.size() > 1)
			{
				msg_box.show(trUtf8("Confirmation"),
												trUtf8("CAUTION: Remove multiple objects at once can cause irreversible invalidations to other objects in the model. Such invalid objects will be deleted too. Do you really want to delete ALL selected objects?"),
												Messagebox::CONFIRM_ICON, Messagebox::YES_NO_BUTTONS);
			}
			else
			{
				if(selected_objects[0]->getObjectType()==OBJ_RELATIONSHIP)
					msg_box.show(trUtf8("Confirmation"),
													trUtf8("CAUTION: Remove a relationship can cause irreversible invalidations to other objects in the model. Such invalid objects will be deleted too. Do you really want to delete the relationship?"),
													Messagebox::CONFIRM_ICON, Messagebox::YES_NO_BUTTONS);
				else
					msg_box.show(trUtf8("Confirmation"),
												trUtf8("Do you really want to delete the selected object?"),
												Messagebox::CONFIRM_ICON, Messagebox::YES_NO_BUTTONS);
			}
		}

		//If the user confirmed the removal or its a cut operation
		if(msg_box.result()==QDialog::Accepted || ModelWidget::cut_operation)
		{
			try
			{
				if(!object)
				{
					itr=selected_objects.begin();
					itr_end=selected_objects.end();

					while(itr!=itr_end)
					{
						object=(*itr);

						//If the object is as FK relationship remove the foreign keys that generates it
						if(object->getObjectType()==BASE_RELATIONSHIP)
						{
							if(object->isProtected())
								throw Exception(QString(Exception::getErrorMessage(ERR_REM_PROTECTED_OBJECT))
																.arg(object->getName(true))
																.arg(object->getTypeName()),
																ERR_REM_PROTECTED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);

							rel=dynamic_cast<BaseRelationship *>(object);
							if(rel->getRelationshipType()==BaseRelationship::RELATIONSHIP_FK)
							{
								aux_table=dynamic_cast<Table *>(rel->getTable(BaseRelationship::DST_TABLE));
								dynamic_cast<Table *>(rel->getTable(BaseRelationship::SRC_TABLE))->getForeignKeys(constrs,false, aux_table);

								if(!rel->isSelfRelationship())
								{
									aux_table=dynamic_cast<Table *>(rel->getTable(BaseRelationship::SRC_TABLE));
									dynamic_cast<Table *>(rel->getTable(BaseRelationship::DST_TABLE))->getForeignKeys(constrs,false, aux_table);
								}

								//Adds the fks to the map of objects to be removed
								while(!constrs.empty())
								{
									tab_obj=constrs.back();
									objs_map[tab_obj->getObjectId()]=tab_obj;
									constrs.pop_back();
								}
							}
						}
						else
						{
							objs_map[object->getObjectId()]=object;
						}
						itr++;
					}

					ritr=objs_map.rbegin();
					ritr_end=objs_map.rend();
					object=nullptr;
          rel=nullptr;
				}

				op_count=op_list->getCurrentSize();
				op_list->startOperationChain();

				do
				{
					if(!object)
					{
						object=ritr->second;
						ritr++;
					}

					obj_type=object->getObjectType();

					//Raises an error if the user try to remove a reserved object
					if(object->isSystemObject())
						throw Exception(Exception::getErrorMessage(ERR_OPR_RESERVED_OBJECT)
														.arg(object->getName()).arg(Utf8String::create(object->getTypeName())),
														ERR_OPR_RESERVED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
					//Raises an error if the user try to remove a protected object
					else if(object->isProtected())
					{
						throw Exception(QString(Exception::getErrorMessage(ERR_REM_PROTECTED_OBJECT))
														.arg(object->getName(true))
														.arg(object->getTypeName()),
														ERR_REM_PROTECTED_OBJECT,__PRETTY_FUNCTION__,__FILE__,__LINE__);
					}
					else if(obj_type!=BASE_RELATIONSHIP)
					{
						tab_obj=dynamic_cast<TableObject *>(object);

						if(tab_obj)
						{
							table=dynamic_cast<BaseTable *>(tab_obj->getParentTable());
							obj_idx=table->getObjectIndex(tab_obj->getName(true), obj_type);

							try
							{
								//If the object is a column validates the column removal before remove it
								if(obj_type==OBJ_COLUMN)
									db_model->validateColumnRemoval(dynamic_cast<Column *>(tab_obj));

								//Register the removed object on the operation list
								op_list->registerObject(tab_obj, Operation::OBJECT_REMOVED, obj_idx, table);
                table->removeObject(obj_idx, obj_type);

								db_model->removePermissions(tab_obj);

								aux_table=dynamic_cast<Table *>(table);
								if(aux_table && obj_type==OBJ_CONSTRAINT &&
									 dynamic_cast<Constraint *>(tab_obj)->getConstraintType()==ConstraintType::foreign_key)
									db_model->updateTableFKRelationships(aux_table);

								table->setModified(true);

								if(aux_table)
									db_model->validateRelationships(tab_obj, aux_table);
							}
							catch(Exception &e)
							{
								throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
							}
						}
						else
						{
							obj_idx=db_model->getObjectIndex(object);

							if(obj_idx >=0 )
							{
								if(obj_type==OBJ_RELATIONSHIP)
								{
									rel=dynamic_cast<BaseRelationship *>(object);
									src_table=rel->getTable(BaseRelationship::SRC_TABLE);
									dst_table=rel->getTable(BaseRelationship::DST_TABLE);
								}

								try
								{
									op_list->registerObject(object, Operation::OBJECT_REMOVED, obj_idx);
									db_model->removeObject(object, obj_idx);
								}
								catch(Exception &e)
								{
									throw Exception(e.getErrorMessage(),e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
								}

								if(rel)
								{
									src_table->setModified(true);
									dst_table->setModified(true);
									rel=nullptr;
									dst_table=src_table=nullptr;
								}
							}
						}
					}
					object=nullptr;
				}
				while(ritr!=ritr_end);

				op_list->finishOperationChain();
				scene->clearSelection();
				this->configurePopupMenu();
				this->modified=true;
				emit s_objectRemoved();
			}
			catch(Exception &e)
			{
				if(e.getErrorType()==ERR_INVALIDATED_OBJECTS)
					op_list->removeOperations();

				if(op_list->isOperationChainStarted())
					op_list->finishOperationChain();

				if(op_count < op_list->getCurrentSize())
				{
					count=op_list->getCurrentSize()-op_count;
					op_list->ignoreOperationChain(true);

					for(unsigned i=0; i < count; i++)
						op_list->removeLastOperation();

					op_list->ignoreOperationChain(false);
				}

				scene->clearSelection();
				emit s_objectRemoved();
				msg_box.show(e);
			}
		}
	}
}

void ModelWidget::appendSQL(void)
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *obj=reinterpret_cast<BaseObject *>(act->data().value<void *>());

	sqlappend_wgt->setAttributes(db_model, obj);
	sqlappend_wgt->show();
	this->modified=(sqlappend_wgt->result()==QDialog::Accepted);
}

void ModelWidget::showObjectMenu(void)
{
	BaseTableView *tab=nullptr;

	/* When the popup is hidden check if there is a table object (colum, constraint, etc) selected,
		 if so, is necessary to reenable the table view deactivated before the menu activation */
	if(this->selected_objects.size()==1)
	{
		//Get the selected table object
		TableObject *tab_obj=dynamic_cast<TableObject *>(this->selected_objects[0]);

		if(tab_obj && tab_obj->getParentTable())
			//Get the graphical representation for table
			tab=dynamic_cast<BaseTableView *>(tab_obj->getParentTable()->getReceiverObject());
	}

	popup_menu.exec(QCursor::pos());

	//If the table object has a parent table
	if(tab)
	{
		//Reacitvates the table
		tab->setEnabled(true);
		//Calls the hoverLeaveEvent in order to hide the child selection
		tab->hoverLeaveEvent(nullptr);
	}
}

void ModelWidget::configureObjectMenu(BaseObject *object)
{
	vector<BaseObject *> vet;
	vet.push_back(object);
	this->configurePopupMenu(vet);
}

void ModelWidget::enableModelActions(bool value)
{
	action_source_code->setEnabled(value);
	action_edit->setEnabled(value);
	action_protect->setEnabled(value);
	action_unprotect->setEnabled(value);
	action_select_all->setEnabled(value);
	action_convert_relnn->setEnabled(value);
	action_deps_refs->setEnabled(value);
	action_new_object->setEnabled(value);
	action_copy->setEnabled(value);
	action_paste->setEnabled(value);
	action_cut->setEnabled(value);
	action_remove->setEnabled(value);
	action_quick_actions->setEnabled(value);
}

void ModelWidget::configureSubmenu(BaseObject *obj)
{
	if(obj)
	{
		if(obj->acceptsOwner() || obj->acceptsSchema())
		{
			QAction *act=nullptr;
			vector<BaseObject *> obj_list;
			map<QString, QAction *> act_map;
			QStringList name_list;
      QMenu *menus[]={ &schemas_menu, &owners_menu, &tags_menu };
      ObjectType types[]={ OBJ_SCHEMA, OBJ_ROLE, OBJ_TAG };

      for(unsigned i=0; i < 3; i++)
			{
				menus[i]->clear();

				if((i==0 && obj->acceptsSchema()) ||
           (i==1 && obj->acceptsOwner()) ||
           (i==2 && (obj->getObjectType()==OBJ_TABLE ||
                     obj->getObjectType()==OBJ_VIEW)))
				{
					obj_list=db_model->getObjects(types[i]);

					if(obj_list.empty())
					{
						menus[i]->addAction(trUtf8("(no objects)"));
						menus[i]->actions().at(0)->setEnabled(false);
					}
					else
					{
						while(!obj_list.empty())
						{
							act=new QAction(Utf8String::create(obj_list.back()->getName()), menus[i]);
							act->setIcon(QPixmap(QString(":/icones/icones/") + BaseObject::getSchemaName(types[i]) + QString(".png")));
							act->setCheckable(true);

							act->setChecked(obj->getSchema()==obj_list.back() ||
                              obj->getOwner()==obj_list.back()  ||
                              ((obj->getObjectType()==OBJ_TABLE ||
                                obj->getObjectType()==OBJ_VIEW) &&
                               dynamic_cast<BaseTable *>(obj)->getTag()==obj_list.back()));

							act->setEnabled(!act->isChecked());
							act->setData(QVariant::fromValue<void *>(obj_list.back()));

							if(i==0)
								connect(act, SIGNAL(triggered(bool)), this, SLOT(moveToSchema(void)));
              else if(i==1)
								connect(act, SIGNAL(triggered(bool)), this, SLOT(changeOwner(void)));
              else
                connect(act, SIGNAL(triggered(bool)), this, SLOT(setTag(void)));

							act_map[obj_list.back()->getName()]=act;
							name_list.push_back(obj_list.back()->getName());
							obj_list.pop_back();
						}

						name_list.sort();
						while(!name_list.isEmpty())
						{
							menus[i]->addAction(act_map[name_list.front()]);
							name_list.pop_front();
						}

						act_map.clear();
					}
				}
			}
		}

		if(obj->getObjectType()!=OBJ_CAST)
		{
			quick_actions_menu.addAction(action_rename);
			action_rename->setData(QVariant::fromValue<void *>(obj));
		}

		if(obj->acceptsSchema())
			quick_actions_menu.addAction(action_moveto_schema);

		if(obj->acceptsOwner())
			quick_actions_menu.addAction(action_change_owner);

    if(obj->getObjectType()==OBJ_TABLE || obj->getObjectType()==OBJ_VIEW)
      quick_actions_menu.addAction(action_set_tag);

		if(Permission::objectAcceptsPermission(obj->getObjectType()))
		{
			quick_actions_menu.addAction(action_edit_perms);
			action_edit_perms->setData(QVariant::fromValue<void *>(obj));
		}

		if(BaseObject::acceptsAppendedSQL(obj->getObjectType()))
		{
			action_append_sql->setData(QVariant::fromValue<void *>(obj));
			quick_actions_menu.addAction(action_append_sql);
		}

		if(!db_model->isProtected() && !quick_actions_menu.isEmpty())
			popup_menu.addAction(action_quick_actions);
	}
}

void ModelWidget::configurePopupMenu(vector<BaseObject *> objects)
{
	QMenu *submenu=nullptr;
	Table *table=nullptr;
	unsigned count, i;
	vector<QMenu *> submenus;
	Constraint *constr=nullptr;
	QAction *action=nullptr;
	TableObject *tab_obj=nullptr;
	QString str_aux;
	bool protected_obj=false, model_protected=db_model->isProtected();

	new_object_menu.clear();
	quick_actions_menu.clear();
	popup_menu.clear();

	this->enableModelActions(false);
	this->selected_objects=objects;
	new_object_menu.setEnabled(!this->db_model->isProtected());

	if(objects.size() <= 1)
	{
		//Case there is no selected object or the selected object is the database model
		if(objects.empty() || (objects.size()==1 && objects[0]==db_model))
		{
			ObjectType types[]={ OBJ_AGGREGATE, OBJ_CAST, OBJ_COLLATION, OBJ_CONVERSION, OBJ_DOMAIN,
													 OBJ_EXTENSION, OBJ_FUNCTION, OBJ_LANGUAGE, OBJ_OPCLASS, OBJ_OPERATOR,
													 OBJ_OPFAMILY, OBJ_RELATIONSHIP, OBJ_ROLE, OBJ_SCHEMA, OBJ_SEQUENCE,
                           OBJ_TABLE, OBJ_TABLESPACE, OBJ_TEXTBOX, OBJ_TYPE, OBJ_VIEW, OBJ_TAG };

			unsigned cnt = sizeof(types)/sizeof(ObjectType);

			//Configures the "New object" menu with the types at database level
			for(i=0; i < cnt; i++)
				new_object_menu.addAction(actions_new_objects[types[i]]);

			action_new_object->setMenu(&new_object_menu);
			popup_menu.addAction(action_new_object);

			configureSubmenu(db_model);

			action_edit->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(db_model)));
			action_source_code->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(db_model)));

			popup_menu.addAction(action_edit);

			popup_menu.addSeparator();
			popup_menu.addAction(action_source_code);

			if(db_model->isProtected())
				popup_menu.addAction(action_unprotect);
			else
				popup_menu.addAction(action_protect);

			if(scene->items().count() > 1)
				popup_menu.addAction(action_select_all);
		}
		else if(objects.size()==1)
		{
			BaseObject *obj=objects[0];
			BaseRelationship *rel=dynamic_cast<BaseRelationship *>(obj);
			ObjectType obj_type=obj->getObjectType(),
					types[]={ OBJ_COLUMN, OBJ_CONSTRAINT, OBJ_INDEX,
										OBJ_RULE, OBJ_TRIGGER },
					sch_types[]={ OBJ_AGGREGATE, OBJ_COLLATION, OBJ_CONVERSION,
												OBJ_DOMAIN, OBJ_EXTENSION, OBJ_FUNCTION, OBJ_OPCLASS,
												OBJ_OPERATOR,	OBJ_OPFAMILY,	OBJ_SEQUENCE,	OBJ_TABLE,
												OBJ_TYPE,	OBJ_VIEW };
			unsigned tab_tp_cnt=sizeof(types)/sizeof(ObjectType),
							 sch_tp_cnt=sizeof(sch_types)/sizeof(ObjectType);

			configureSubmenu(obj);
			popup_menu.addAction(action_edit);

			if((obj_type==OBJ_SCHEMA && obj->isSystemObject()) ||
				 (!obj->isProtected() && (obj_type==OBJ_TABLE || obj_type==BASE_RELATIONSHIP ||
																	obj_type==OBJ_RELATIONSHIP || obj_type==OBJ_SCHEMA)))
			{
				if(obj_type==OBJ_TABLE)
				{
					for(i=0; i < tab_tp_cnt; i++)
						new_object_menu.addAction(actions_new_objects[types[i]]);
					action_new_object->setMenu(&new_object_menu);
					popup_menu.insertAction(action_quick_actions, action_new_object);
				}
				else if(obj_type==OBJ_RELATIONSHIP || obj_type==BASE_RELATIONSHIP)
				{

					if(obj_type==OBJ_RELATIONSHIP)
					{
						for(i=0; i < 2; i++)
							new_object_menu.addAction(actions_new_objects[types[i]]);

						action_new_object->setMenu(&new_object_menu);
						popup_menu.insertAction(action_quick_actions, action_new_object);
					}

					if(rel->getRelationshipType()==Relationship::RELATIONSHIP_NN)
					{
						action_convert_relnn->setData(QVariant::fromValue<void *>(rel));
						popup_menu.addAction(action_convert_relnn);
					}

					if(!rel->isSelfRelationship())
					{
						if(rel->getPoints().empty())
						{
							action_break_rel_line->setData(QVariant::fromValue<void *>(rel));
							popup_menu.addAction(action_break_rel_line);
						}
						else
						{
							action_remove_rel_points->setData(QVariant::fromValue<void *>(rel));
							popup_menu.addAction(action_remove_rel_points);
						}
					}
				}
				else if(obj_type == OBJ_SCHEMA)
				{
					for(i=0; i < sch_tp_cnt; i++)
						new_object_menu.addAction(actions_new_objects[sch_types[i]]);
					action_new_object->setMenu(&new_object_menu);
					popup_menu.insertAction(action_quick_actions, action_new_object);

					popup_menu.addAction(action_sel_sch_children);
					action_sel_sch_children->setData(QVariant::fromValue<void *>(obj));
				}
			}

			/* Adding the action to highlight the object only when the sender is not one of the
			the objects that calls this method from inside the ModelWidget instance. This action
			is mainly used when the user wants to find a graphical object from the ModelObjects dockwidget*/
			if((sender()!=this && sender()!=scene) && dynamic_cast<BaseGraphicObject *>(obj))
			{
				popup_menu.addAction(action_highlight_object);
				action_highlight_object->setData(QVariant::fromValue<void *>(obj));
			}

			action_edit->setData(QVariant::fromValue<void *>(obj));
			action_source_code->setData(QVariant::fromValue<void *>(obj));
			action_deps_refs->setData(QVariant::fromValue<void *>(obj));
			tab_obj=dynamic_cast<TableObject *>(obj);


			if(tab_obj &&  tab_obj->getObjectType()==OBJ_COLUMN)
			{
				Column *col=dynamic_cast<Column *>(tab_obj);

				if(tab_obj->isAddedByRelationship())
				{
					action_parent_rel->setData(QVariant::fromValue<void *>(dynamic_cast<Column *>(tab_obj)->getParentRelationship()));
					popup_menu.addAction(action_parent_rel);
				}
				else if(col->getType().isSerialType())
				{
					action_create_seq_col->setData(QVariant::fromValue<void *>(col));
					popup_menu.addAction(action_create_seq_col);
				}
			}

			popup_menu.addSeparator();
			popup_menu.addAction(action_source_code);

			if(!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship()))
				popup_menu.addAction(action_deps_refs);
		}
	}

	/* Adds the protect/unprotect action when the selected object was not included by relationship
	and if its a table object and the parent table is not protected. */
	if(!objects.empty() &&
		 !this->db_model->isProtected() &&
		 (!tab_obj || (tab_obj && !tab_obj->getParentTable()->isProtected() && !tab_obj->isAddedByRelationship())))
	{
		if(!objects[0]->isProtected())
			popup_menu.addAction(action_protect);
		else
			popup_menu.addAction(action_unprotect);

		popup_menu.addSeparator();
	}

	//Adding the copy and paste if there is selected objects
	if(!model_protected &&
		 !(objects.size()==1 && (objects[0]==db_model || objects[0]->getObjectType()==BASE_RELATIONSHIP)) &&
		 !objects.empty() && (!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship())))
	{
		popup_menu.addAction(action_copy);

		count=objects.size();
		i=0;
		while(i < count && !protected_obj)
			protected_obj=objects[i++]->isProtected();

		popup_menu.addAction(action_cut);
	}

	//If there is copied object adds the paste action
	if(!model_protected && !copied_objects.empty())
		popup_menu.addAction(action_paste);

	/* Adding the delete object action. This action will be unavailable on following conditions:
	1) The selected object is the database itself
	2) The object is protected
	3) The object is table child object and it was added by relationship
	4) The object is a base relationship (table-view) */
	if((tab_obj && !tab_obj->isAddedByRelationship() && !tab_obj->isProtected()) ||
		 (objects.size()==1 && objects[0]->isProtected()) ||
		 (!tab_obj && objects.size()==1 && objects[0]!=db_model && objects[0]->getObjectType()!=BASE_RELATIONSHIP) ||
		 (objects.size()==1 && objects[0]->getObjectType()==BASE_RELATIONSHIP &&
			dynamic_cast<BaseRelationship *>(objects[0])->getRelationshipType()==BaseRelationship::RELATIONSHIP_FK) ||
		 objects.size() > 1)
		popup_menu.addAction(action_remove);

	//If the table object is a column creates a special menu to acess the constraints that is applied to the column
	if(tab_obj)
	{
		table=dynamic_cast<Table *>(tab_obj->getParentTable());

		if(tab_obj->getObjectType()==OBJ_COLUMN)
		{
			count=table->getConstraintCount();

			for(i=0; i < count; i++)
			{
				constr=table->getConstraint(i);
				if(constr->isColumnReferenced(dynamic_cast<Column *>(tab_obj), false))
				{
					switch(!constr->getConstraintType())
					{
						case ConstraintType::primary_key: str_aux=QString("_%1").arg(TableObjectView::TXT_PRIMARY_KEY); break;
						case ConstraintType::foreign_key: str_aux=QString("_%1").arg(TableObjectView::TXT_FOREIGN_KEY); break;
						case ConstraintType::check: str_aux=QString("_%1").arg(TableObjectView::TXT_CHECK); break;
						case ConstraintType::unique: str_aux=QString("_%1").arg(TableObjectView::TXT_UNIQUE); break;
						case ConstraintType::exclude: str_aux=QString("_%1").arg(TableObjectView::TXT_EXCLUDE); break;
					}

					//For each constaint is created a menu with the edit, source code, protect/unprotect and delete actions
					submenu=new QMenu(&popup_menu);
					submenu->setIcon(QPixmap(QString(":/icones/icones/") +
																	 BaseObject::getSchemaName(OBJ_CONSTRAINT) + str_aux + QString(".png")));
					submenu->setTitle(Utf8String::create(constr->getName()));

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(QPixmap(QString(":/icones/icones/editar.png")));
					action->setText(trUtf8("Properties"));
					action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
					connect(action, SIGNAL(triggered(bool)), this, SLOT(editObject(void)));
					submenu->addAction(action);

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(QPixmap(QString(":/icones/icones/codigosql.png")));
					action->setText(trUtf8("Source code"));
					action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
					connect(action, SIGNAL(triggered(bool)), this, SLOT(showSourceCode(void)));
					submenu->addAction(action);

					if(!constr->isAddedByRelationship())
					{
						if(!constr->getParentTable()->isProtected())
						{
							action=new QAction(dynamic_cast<QObject *>(&popup_menu));
							action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
							connect(action, SIGNAL(triggered(bool)), this, SLOT(protectObject(void)));
							submenu->addAction(action);

							if(constr->isProtected())
							{
								action->setIcon(QPixmap(QString(":/icones/icones/desbloqobjeto.png")));
								action->setText(trUtf8("Unprotect"));
							}
							else
							{
								action->setIcon(QPixmap(QString(":/icones/icones/bloqobjeto.png")));
								action->setText(trUtf8("Protect"));
							}
						}

						action=new QAction(dynamic_cast<QObject *>(submenu));
						action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
						action->setIcon(QPixmap(QString(":/icones/icones/excluir.png")));
						action->setText(trUtf8("Delete"));
						connect(action, SIGNAL(triggered(bool)), this, SLOT(removeObjects(void)));
						submenu->addAction(action);
					}
					submenus.push_back(submenu);
				}
			}

			//Adding the constraint submenus to the main popup menu
			if(!submenus.empty())
			{
				submenu=new QMenu(&popup_menu);
				submenu->setTitle(trUtf8("Constraints"));
				submenu->setIcon(QPixmap(QString(":/icones/icones/") +
																 BaseObject::getSchemaName(OBJ_CONSTRAINT) + QString("_grp.png")));
				count=submenus.size();
				for(i=0; i < count; i++)
					submenu->addMenu(submenus[i]);

				popup_menu.insertMenu(action_edit, submenu);
			}
		}
	}

	//Enable the popup actions that are visible
	QList<QAction *> actions=popup_menu.actions();
	actions.append(quick_actions_menu.actions());
	while(!actions.isEmpty())
	{
		actions.back()->setEnabled(true);
		actions.pop_back();
	}
}

bool ModelWidget::isModified(void)
{
	return(modified);
}

DatabaseModel *ModelWidget::getDatabaseModel(void)
{
	return(db_model);
}

OperationList *ModelWidget::getOperationList(void)
{
	return(op_list);
}

void ModelWidget::highlightObject(void)
{
	QAction *action=dynamic_cast<QAction *>(sender());

	if(action )
	{
		BaseObject *obj=reinterpret_cast<BaseObject *>(action->data().value<void *>());
		BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(obj);

		if(graph_obj)
		{
			BaseObjectView *obj_view=dynamic_cast<BaseObjectView *>(graph_obj->getReceiverObject());

			scene->clearSelection();
			obj_view->setSelected(true);
			viewport->centerOn(obj_view);
		}
	}
}

void ModelWidget::createSequenceForColumn(void)
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		Column *col=reinterpret_cast<Column *>(action->data().value<void *>());
		Sequence *seq=nullptr;
		Table *tab=dynamic_cast<Table *>(col->getParentTable());

		op_list->startOperationChain();
		op_list->registerObject(col, Operation::OBJECT_MODIFIED, -1, tab);

		//Creates a sequence which name is like the ones auto generated by PostgreSQL
		seq=new Sequence;
		seq->setName(BaseObject::formatName(tab->getName() + "_" + col->getName() + "_seq"));
		seq->setSchema(tab->getSchema());
		seq->setDefaultValues(col->getType());
		seq->setOwnerColumn(col);

		//Changes the column type to the alias for serial type
		col->setType(col->getType().getAliasType());
		col->setNotNull(true);

		//Clean up the column's default value since it'll be set when the sequence is created
		col->setDefaultValue("");

		op_list->registerObject(seq, Operation::OBJECT_CREATED);
		db_model->addSequence(seq);
		op_list->finishOperationChain();

		//Revalidate the relationships since the modified column can be a primary key
		if(tab->getPrimaryKey()->isColumnReferenced(col))
			db_model->validateRelationships();

		tab->setModified(true);
		this->setModified(true);
		emit s_objectCreated();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::breakRelationshipLine(void)
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		BaseRelationship *rel=dynamic_cast<BaseRelationship *>(selected_objects[0]);
		BaseTableView *src_tab=reinterpret_cast<BaseTableView *>(rel->getTable(BaseRelationship::SRC_TABLE)->getReceiverObject()),
									*dst_tab=reinterpret_cast<BaseTableView *>(rel->getTable(BaseRelationship::DST_TABLE)->getReceiverObject());
		float dx, dy;
		unsigned break_type=action->data().toUInt();

		op_list->registerObject(rel, Operation::OBJECT_MODIFIED);

		if(break_type==BREAK_VERT_NINETY_DEGREES)
			rel->setPoints({ QPointF(src_tab->getCenter().x(), dst_tab->getCenter().y()) });
		else if(break_type==BREAK_HORIZ_NINETY_DEGREES)
			rel->setPoints({ QPointF(dst_tab->getCenter().x(), src_tab->getCenter().y()) });
		else if(break_type==BREAK_HORIZ_2NINETY_DEGREES)
		{
			//Calculates the midle vertical point between the tables centers
			dy=(src_tab->getCenter().y() + dst_tab->getCenter().y())/2;

			//Adds two points on the middle space between tables creating two 90° angles
			rel->setPoints({ QPointF(src_tab->getCenter().x(), dy),
											 QPointF(dst_tab->getCenter().x(), dy) });
		}
		else
		{
			//Calculates the middle horizontal point between the tables centers
			dx=(src_tab->getCenter().x() + dst_tab->getCenter().x())/2;

			//Adds two points on the middle space between tables creating two 90° angles
			rel->setPoints({ QPointF(dx, src_tab->getCenter().y()),
											 QPointF(dx, dst_tab->getCenter().y()) });
		}

		rel->setModified(true);
		this->setModified(true);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::removeRelationshipPoints(void)
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		BaseRelationship *rel=reinterpret_cast<BaseRelationship *>(action->data().value<void *>());

		op_list->registerObject(rel, Operation::OBJECT_MODIFIED);
		rel->setPoints({});
		scene->clearSelection();

		rel->setModified(true);
		this->setModified(true);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorType(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void ModelWidget::rearrangeSchemas(QPointF origin, unsigned tabs_per_row, unsigned sch_per_row, float obj_spacing)
{
	vector<BaseObject *>::iterator itr, itr_end;
	Schema *schema=nullptr;
	SchemaView *sch_view=nullptr;
	unsigned sch_id=0;
	float x=origin.x(), y=origin.y(), max_y=-1, cy=0;

	itr=db_model->getObjectList(OBJ_SCHEMA)->begin();
	itr_end=db_model->getObjectList(OBJ_SCHEMA)->end();

	while(itr!=itr_end)
	{
		schema=dynamic_cast<Schema *>(*itr);

		/* Forcing the schema rectangle to be visible in order to correctly positioin
			 schemas over the screen */
		schema->setRectVisible(true);

		sch_view=dynamic_cast<SchemaView *>(schema->getReceiverObject());
		schema->setModified(true);

		//The schema is processed only there are tables inside of it
		if(sch_view->getChildrenCount() > 0)
		{
			//Organizing the tables inside the schema
			rearrangeTables(schema, QPointF(x,y), tabs_per_row, obj_spacing);
			schema->setModified(true);

			cy=sch_view->pos().y() + sch_view->boundingRect().height();

			//Defining the maximum y position to avoid schema boxes colliding vertically
			if(max_y < cy)
				max_y=cy;

			sch_id++;

			//It the current schema is the last of it`s row
			if(sch_id >= sch_per_row)
			{
				//Incrementing the row position
				sch_id=0;
				y=max_y + obj_spacing;
				x=origin.x();
				max_y=-1;
			}
			else
				//Configuring the x position for the next schema on the current row
				x=sch_view->pos().x() + sch_view->boundingRect().width() + obj_spacing;
		}

		itr++;
	}

	//Adjust the whole scene size due to table/schema repositioning
	this->adjustSceneSize();
}

void ModelWidget::rearrangeTables(Schema *schema, QPointF origin, unsigned tabs_per_row, float obj_spacing)
{
	if(schema)
	{
		vector<BaseObject *> tables, views;
		vector<BaseObject *>::iterator itr;
		BaseTableView *tab_view=nullptr;
		BaseTable *base_tab=nullptr;
		unsigned tab_id=0;
		float max_y=-1, x=origin.x(), y=origin.y(), cy=0;

		//Get the tables and views for the specified schema
		tables=db_model->getObjects(OBJ_TABLE, schema);
		views=db_model->getObjects(OBJ_VIEW, schema);
		tables.insert(tables.end(), views.begin(), views.end());

		itr=tables.begin();
		while(itr!=tables.end())
		{
			base_tab=dynamic_cast<BaseTable *>(*itr);
			tab_view=dynamic_cast<BaseTableView *>(base_tab->getReceiverObject());			
			tab_view->setPos(QPointF(x,y));

			//Defining the maximum y position to avoid table boxes colliding vertically
			cy=tab_view->pos().y()  + tab_view->boundingRect().bottomRight().y();
			if(max_y < cy)
				max_y=cy;

			//It the current table is the last of it's row
			tab_id++;
			if(tab_id >= tabs_per_row)
			{
				//Incrementing the row position
				tab_id=0;
				y=max_y + obj_spacing;
				x=origin.x();
				max_y=-1;
			}
			else
				//Configuring the x position for the next table on the current row
				x=tab_view->pos().x() + tab_view->boundingRect().width() + obj_spacing;

			itr++;
		}
	}
}
