void QuitProc P((GtkObject *object, gpointer user_data));
void IcsClientProc P((GtkObject *object, gpointer user_data));
void MachineBlackProc P((GtkObject *object, gpointer user_data));
void MachineWhiteProc P((GtkObject *object, gpointer user_data));
void TwoMachinesProc P((GtkObject *object, gpointer user_data));
void AcceptProc P((GtkObject *object, gpointer user_data));
void DeclineProc P((GtkObject *object, gpointer user_data));
void RematchProc P((GtkObject *object, gpointer user_data));
void CallFlagProc P((GtkObject *object, gpointer user_data));
void Drawroc P((GtkObject *object, gpointer user_data));
void AbortProc P((GtkObject *object, gpointer user_data));
void AdjournProc P((GtkObject *object, gpointer user_data));
void ResignProc P((GtkObject *object, gpointer user_data));
void StopObservingProc P((GtkObject *object, gpointer user_data));
void StopExaminingProc P((GtkObject *object, gpointer user_data));
void AdjuWhiteProc P((GtkObject *object, gpointer user_data));
void AdjuBlackProc P((GtkObject *object, gpointer user_data));
void AdjuDrawProc P((GtkObject *object, gpointer user_data));
void ResetProc P((GtkObject *object, gpointer user_data));
void WhiteClockProc P((GtkObject *object, gpointer user_data));
void BlackClockProc P((GtkObject *object, gpointer user_data));

/* Step Menu */
void BackwardProc P((GtkObject *object, gpointer user_data));
void ForwardProc P((GtkObject *object, gpointer user_data));
void ToStartProc P((GtkObject *object, gpointer user_data));
void ToEndProc P((GtkObject *object, gpointer user_data));
void RevertProc P((GtkObject *object, gpointer user_data));
void TruncateGameProc P((GtkObject *object, gpointer user_data));
void MoveNowProc P((GtkObject *object, gpointer user_data));
void RetractMoveProc P((GtkObject *object, gpointer user_data));

/* Option Menu */
void ShowThinkingProc P((GtkObject *object, gpointer user_data));
void HideThinkingProc P((GtkObject *object, gpointer user_data));
void FlipViewProc P((GtkObject *object, gpointer user_data));
void GetMoveListProc P((GtkObject *object, gpointer user_data));

/* Help Menu */

void InfoProc  P((GtkObject *object, gpointer user_data));
void ManProc  P((GtkObject *object, gpointer user_data));
void HintProc  P((GtkObject *object, gpointer user_data));
void BookProc  P((GtkObject *object, gpointer user_data));
void AboutProc  P((GtkObject *object, gpointer user_data));


void ShowCoordsProc P((GtkObject *object, gpointer user_data));
void ErrorPopDownProc P((GtkObject *object, gpointer user_data));
void PauseProc P((GtkObject *object, gpointer user_data));
void LoadNextGameProc P((GtkObject *object, gpointer user_data));
void LoadPrevGameProc P((GtkObject *object, gpointer user_data));
void ReloadGameProc P((GtkObject *object, gpointer user_data));
void EventProc P((GtkWindow *window, GdkEvent *event, gpointer data));
void UserMoveProc P((GtkWindow *window, GdkEvent *event, gpointer data));
gboolean CloseWindowProc P((GtkWidget *button));
