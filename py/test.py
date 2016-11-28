import json

from github import Github


class TrelloData(object):
    def __init__(self, filename):
        self.trelloData = json.load(open(filename, 'r'))

    def listID(self, listname):
        for ls in self.trelloData['lists']:
            if ls['name'] == listname:
                return ls['id']

    def labelID(self, labelname):
        for lb in self.trelloData['labels']:
            if lb['name'] == labelname:
                return lb['id']

    def __cards(self, id):
        for card in self.trelloData['cards']:
            if card['idList'] == id:
                yield card

    def cards(self, listname):
        listid = self.listID(listname)
        return self.__cards(listid)


if __name__ == '__main__':

    trello = TrelloData('trello.json')
    lbid = trello.labelID('Napkin')
    print(lbid)
    for card in trello.cards('Tasks'):
        if lbid in card['idLabels']:
            print(card['name'])
            # print(card['desc'])

    # g = Github('5278d52cadd45681753529a55fc2ad37f920bc07')
    #
    # for repo in g.get_user().get_repos():
    #     if repo.name == 'napkinpy':
    #         break
    #
    # # repo.create_issue('Added from python', 'And some more in the body')
    # print(repo.name)